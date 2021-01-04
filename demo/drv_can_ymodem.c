/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-29     redoc        the first version
 */

#include "board.h"
#include "thread_can.h"
#include "can_ymodem.h"
#include "drv_can_ymodem.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.can_ymodem"
#include <drv_log.h>

#define CAN_ID_KEYBOARD0_TO_SC_FILE   (0x30)

struct rt_can_ymodem_device can_ymodem_device;

static rt_err_t can_file_send(uint32_t can_id, can_cmd_file_t cmd, uint8_t *buffer, uint8_t size);

static rt_err_t can_file_send(uint32_t can_id, can_cmd_file_t cmd, uint8_t *buffer, uint8_t size)
{
    uint8_t *pdata = rt_calloc(1, 8);
    
    if(0 == size)
    {
        log_e("can file send size can't be zero!");
        goto __exit;
    }
    
    *pdata = cmd;
    rt_memcpy(pdata + CAN_FILE_CMD_SIZE, buffer, size);

    can_send(can_id, pdata, size + CAN_FILE_CMD_SIZE);
    
    rt_free(pdata);
    
    return RT_EOK;
    
__exit:
    
    rt_free(pdata);
    
    return -RT_ERROR;
}


static int rt_hw_can_ymodem_send(struct rt_can_ymodem_device *can_ymodem,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    if(CAN_ID_KEYBOARD0_TO_SC_FILE == can_ymodem->channel)
    {
        can_file_send(can_ymodem->channel, CAN_CMD_FILE_FILE_SLAVE, (uint8_t*)buffer, size);
    }

    return RT_EOK;
}    

void drv_can_ymodem_recv(uint32_t ch, const void *buffer, rt_size_t size)
{
    if(CAN_ID_KEYBOARD0_TO_SC_FILE == ch)
    {
        rt_can_ymodem_rx(&can_ymodem_device, buffer, size);
    }
}

static const struct rt_can_ymodem_ops topctrl_can_ymodem_ops =
{
    .puts = rt_hw_can_ymodem_send,
};

int rt_hw_can_ymodem_init(void)
{
    rt_err_t result = 0;
    
    can_ymodem_device.ops = &topctrl_can_ymodem_ops;
    
    can_ymodem_device.channel = CAN_ID_KEYBOARD0_TO_SC_FILE;
    
    result = rt_hw_can_ymodem_register(&can_ymodem_device, "can_ymodem", RT_DEVICE_FLAG_RDWR, RT_NULL);
    
    return result;
}
INIT_DEVICE_EXPORT(rt_hw_can_ymodem_init);
