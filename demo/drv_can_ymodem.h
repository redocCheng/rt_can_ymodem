/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-29     redoc         first version.
 */

#ifndef __DRV_CAN_YMODEM_H__
#define __DRV_CAN_YMODEM_H__

#include <rtthread.h>

typedef enum
{
    CAN_CMD_FILE_CMD_MASTER = 0,
    CAN_CMD_FILE_CMD_SLAVE,
    CAN_CMD_FILE_FILE_MASTER,
    CAN_CMD_FILE_FILE_SLAVE,
    
}can_cmd_file_t;

void drv_can_ymodem_recv(uint32_t ch, const void *buffer, rt_size_t size);

#endif
