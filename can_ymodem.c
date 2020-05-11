/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-29     redoc        the first version
 */
 
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "can_ymodem.h"


static void rt_hw_can_ymodem_rx(struct rt_can_ymodem_device *can_ymodem,
                         const rt_uint8_t    *buffer,
                         rt_size_t         size)
{
    rt_base_t level;
    struct rt_serial_rx_fifo* rx_fifo;
    int ch = -1;
    int length;
    
    /* interrupt mode receive */
    rx_fifo = (struct rt_serial_rx_fifo*)can_ymodem->can_ymodem_rx;
    RT_ASSERT(rx_fifo != RT_NULL);
    length = 0;
    
    while (length < size)
    {
        ch = buffer[length];
        length ++;
        
        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        rx_fifo->buffer[rx_fifo->put_index] = ch;
        
        rx_fifo->put_index += 1;
        
        if (rx_fifo->put_index >= RT_CAN_YMODEM_DEVICE_SIZE) rx_fifo->put_index = 0;

        /* if the next position is read index, discard this 'read char' */
        if (rx_fifo->put_index == rx_fifo->get_index)
        {
            rx_fifo->get_index += 1;
            rx_fifo->is_full = RT_TRUE;
            if (rx_fifo->get_index >= RT_CAN_YMODEM_DEVICE_SIZE) rx_fifo->get_index = 0;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(level);
    }

    /* invoke callback */
    if (can_ymodem->parent.rx_indicate != RT_NULL)
    {
        rt_size_t rx_length;

        /* get rx length */
        level = rt_hw_interrupt_disable();
        rx_length = (rx_fifo->put_index >= rx_fifo->get_index)? (rx_fifo->put_index - rx_fifo->get_index):
            (RT_CAN_YMODEM_DEVICE_SIZE - (rx_fifo->get_index - rx_fifo->put_index));
        rt_hw_interrupt_enable(level);

        if (rx_length)
        {
            can_ymodem->parent.rx_indicate(&can_ymodem->parent, rx_length);
        }
    }
}

void rt_can_ymodem_rx(struct rt_can_ymodem_device *can_ymodem, const void  *buffer, rt_size_t size)
{
    rt_hw_can_ymodem_rx(can_ymodem, buffer, size);
}

rt_inline int _can_ymodem_rx(struct rt_can_ymodem_device *can_ymodem, rt_uint8_t *data, int length)
{
    int size;
    struct rt_can_ymodem_rx_fifo* rx_fifo;

    RT_ASSERT(can_ymodem != RT_NULL);
    size = length;

    rx_fifo = (struct rt_can_ymodem_rx_fifo*) can_ymodem->can_ymodem_rx;
    RT_ASSERT(rx_fifo != RT_NULL);

    /* read from software FIFO */
    while (length)
    {
        int ch;
        rt_base_t level;

        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == RT_FALSE))
        {
            /* no data, enable interrupt and break out */
            rt_hw_interrupt_enable(level);
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= RT_CAN_YMODEM_DEVICE_SIZE) rx_fifo->get_index = 0;

        if (rx_fifo->is_full == RT_TRUE)
        {
            rx_fifo->is_full = RT_FALSE;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        *data = ch & 0xff;
        data ++; length --;
    }

    return size - length;
}

rt_inline int _can_ymodem_tx(struct rt_can_ymodem_device *can_ymodem, rt_uint8_t *data, int length)
{
    int size;
    RT_ASSERT(can_ymodem != RT_NULL);
    int len;
    
    size = length;
    while (length)
    {
        if(length > RT_CAN_YMODEM_PACK_SIZE_TX)
        {
            len = RT_CAN_YMODEM_PACK_SIZE_TX;
        }
        else
        {
            len = length;
        }

        can_ymodem->ops->puts(can_ymodem, data, len);
        
        data += len;
        length -= len;
        
        rt_thread_mdelay(10);
    }

    return size - length;
}

static rt_err_t rt_can_ymodem_init(struct rt_device *dev)
{
    struct rt_can_ymodem_device *can_ymodem;

    RT_ASSERT(dev != RT_NULL);
    can_ymodem = (struct rt_can_ymodem_device *)dev;

    /* initialize rx/tx */
    can_ymodem->can_ymodem_rx = RT_NULL;

    return RT_EOK;
}

static rt_err_t rt_can_ymodem_open(struct rt_device *dev, rt_uint16_t oflag)
{
    struct rt_can_ymodem_device *can_ymodem;
    RT_ASSERT(dev != RT_NULL);
    can_ymodem = (struct rt_can_ymodem_device *)dev;
    
    /* get open flags */
    dev->open_flag = oflag & 0xff;
    
    if (oflag & RT_DEVICE_FLAG_RDWR)
    {
        if(can_ymodem->can_ymodem_rx == RT_NULL)
        {
            struct rt_can_ymodem_rx_fifo* rx_fifo;
            
            rx_fifo = (struct rt_can_ymodem_rx_fifo*) rt_malloc (sizeof(struct rt_can_ymodem_rx_fifo) +
                RT_CAN_YMODEM_DEVICE_SIZE);
            RT_ASSERT(rx_fifo != RT_NULL);
            rx_fifo->buffer = (rt_uint8_t*) (rx_fifo + 1);
            rt_memset(rx_fifo->buffer, 0, RT_CAN_YMODEM_DEVICE_SIZE);
            rx_fifo->put_index = 0;
            rx_fifo->get_index = 0;
            rx_fifo->is_full = RT_FALSE;

            can_ymodem->can_ymodem_rx = rx_fifo;
        }
    }

    return RT_EOK;
}

static rt_err_t rt_can_ymodem_close(struct rt_device *dev)
{
    struct rt_can_ymodem_device *can_ymodem;
    RT_ASSERT(dev != RT_NULL);
    can_ymodem = (struct rt_can_ymodem_device *)dev;
    
    if (dev->open_flag & RT_DEVICE_FLAG_RDWR)
    {
        struct rt_can_ymodem_rx_fifo* rx_fifo;

        rx_fifo = (struct rt_can_ymodem_rx_fifo*)can_ymodem->can_ymodem_rx;
        RT_ASSERT(rx_fifo != RT_NULL);

        rt_free(rx_fifo);
        can_ymodem->can_ymodem_rx = RT_NULL;

        dev->open_flag &= ~RT_DEVICE_FLAG_RDWR;
    }
    

    return RT_EOK;
}

static rt_size_t rt_can_ymodem_read(struct rt_device *dev,
                                rt_off_t          pos,
                                void             *buffer,
                                rt_size_t         size)
{
    struct rt_can_ymodem_device *can_ymodem;
    RT_ASSERT(dev != RT_NULL);
    
    if (size == 0) 
    {   
        return 0;
    }
    can_ymodem = (struct rt_can_ymodem_device *)dev;
    
    return _can_ymodem_rx(can_ymodem, buffer, size);
}

static rt_size_t rt_can_ymodem_write(struct rt_device *dev,
                                 rt_off_t          pos,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    struct rt_can_ymodem_device *can_ymodem;
    RT_ASSERT(dev != RT_NULL);
    
    if (size == 0) 
    {   
        return 0;
    }
    can_ymodem = (struct rt_can_ymodem_device *)dev;
    
    return _can_ymodem_tx(can_ymodem, (rt_uint8_t*)buffer, size);
}

static rt_err_t rt_can_ymodem_control(struct rt_device *dev,
                                  int              cmd,
                                  void             *args)
{
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops can_ymodem_ops = 
{
    rt_can_ymodem_init,
    rt_can_ymodem_open,
    rt_can_ymodem_close,
    rt_can_ymodem_read,
    rt_can_ymodem_write,
    rt_can_ymodem_control
};
#endif

rt_err_t rt_hw_can_ymodem_register(struct rt_can_ymodem_device *can_ymodem,
                                   const char              *name,
                                   rt_uint32_t              flag,
                                   void                    *data)
{
    rt_err_t ret;
    struct rt_device *device;
    RT_ASSERT(can_ymodem != RT_NULL);

    device = &(can_ymodem->parent);

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &can_ymodem_ops;
#else
    device->init        = rt_can_ymodem_init;
    device->open        = rt_can_ymodem_open;
    device->close       = rt_can_ymodem_close;
    device->read        = rt_can_ymodem_read;
    device->write       = rt_can_ymodem_write;
    device->control     = rt_can_ymodem_control;
#endif
    device->user_data   = data;

    /* register a character device */
    ret = rt_device_register(device, name, flag);

    return ret;
}

