/*
 * Copyright (c) 2019, Anke Development Team
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

struct rt_can_ymodem_device can_ymodem_device;

/*  fun of can send  */
static int rt_hw_can_ymodem_topctrl_to_ctrlbox(struct rt_can_ymodem_device *can_ymodem,
                                const void       *buffer,
                                rt_size_t         size)
{
    // can_send(id, cmd, buffer, size)
}    

/*  recv ymodem msg  */
void drv_can_ymodem_ctrlbox_to_topctrl(const void  *buffer, rt_size_t size)
{
    rt_can_ymodem_rx(&can_ymodem_device, buffer, size);
}

static const struct rt_can_ymodem_ops topctrl_can_ymodem_ops =
{
    .puts = rt_hw_can_ymodem_topctrl_to_ctrlbox,
};

int rt_hw_can_ymodem_init(void)
{
    rt_err_t result = 0;
    
    can_ymodem_device.ops = &topctrl_can_ymodem_ops;
    
    result = rt_hw_can_ymodem_register(&can_ymodem_device, "can_ymodem", RT_DEVICE_FLAG_RDWR, RT_NULL);
    
    return result;
}
INIT_DEVICE_EXPORT(rt_hw_can_ymodem_init);
