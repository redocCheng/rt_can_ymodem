#include <rthw.h>
#include <rtthread.h>
#include <shell.h>
#include <finsh.h>
#include <ulog.h>
#include <easyflash.h>
#include <ymodem.h>
#include <board.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include "thread_can.h"

static char log_name_buf[32] = {0};
static int fd;
static uint8_t log_export_flag = false;
static size_t log_file_total_size ,log_file_cur_size;

static rt_err_t log_file_open(char *file_name)
{
    static time_t now;
    static struct tm *tm, tm_tmp;
    uint32_t size;
    
    rt_err_t result = RT_EOK;

    now = time(NULL);
    tm = gmtime_r(&now, &tm_tmp);
    
//  size = rt_snprintf(log_name_buf,32, "/topctrl_log_%04d%02d%02d.txt",tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    
    size = rt_snprintf(log_name_buf,32, "/%s", file_name);
    
    rt_kprintf("%s\r\n",log_name_buf);
    
    fd = open(log_name_buf, O_WRONLY | O_CREAT);
    
    if (fd >= 0)
    {
        write(fd, log_name_buf, size);
        write(fd, ":\r\n\r\n", sizeof(":\r\n\r\n"));
        rt_kprintf("open done.\n");

    }
    else
    {
        result = -RT_ERROR;
    }
    
    return result;
}

static void log_out_print(size_t size, size_t total_size)
{
    static uint8_t per_printf = 0;
    
    float per = size * 100 / total_size;
    
    if(per - per_printf >= 10)
    {
        per_printf = per;
        rt_kprintf("log output %3d%\r\n",per_printf);
    }
}

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) {
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *) &buf[0];
    file_size = (char *) &buf[rt_strlen(file_name) + 1];
    log_file_total_size = atol(file_size);
    log_file_cur_size = 0;
    
    rt_kprintf("log name %s  size %dKB\r\n",file_name , log_file_total_size);
    
    /* erase backup section */
    if (RT_EOK != log_file_open(file_name)) 
    {
        /* if erase fail then end session */
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) {
    
    if (log_file_cur_size + len < log_file_total_size)
    {
        write(fd, buf, len);

        log_file_cur_size += len;
    }
    else
    {
        write(fd, buf,log_file_total_size - log_file_cur_size);
    }  
    log_out_print(log_file_cur_size, log_file_total_size);

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_end(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    close(fd);
    rt_kprintf("topctrl log out succ!!\r\n");
}

void topctrl_log_be(uint8_t argc, char **argv) {
    char new_char[2], c_file_size[11] = { 0 }, c_crc32_checksum[11] = { 0 };
    struct rym_ctx rctx;
    rt_device_t dev;
    
    dev = rt_device_find("can_ymodem");
    
    fd = -1;

    rt_kprintf("Waring: This operator will not recovery. If you want, press 'Y'.\n");
    new_char[0] = getchar();
    rt_kprintf("%c", new_char[0]);
    new_char[1] = getchar();
    rt_kprintf("%c", new_char[1]);
    if ((new_char[0] != 'y') && (new_char[0] != 'Y')) {
        goto __exit;
    }
    rt_kprintf("Please select the application firmware file and use Ymodem to send.\n");


    if (!rym_recv_on_device(&rctx, dev, RT_DEVICE_OFLAG_RDWR ,
            ymodem_on_begin, ymodem_on_data, ymodem_on_end, RT_TICK_PER_SECOND)) {
        /* wait some time for terminal response finish */
        rt_kprintf("log out succ.\n");
        
    } else {
        /* wait some time for terminal response finish */
      
        rt_kprintf("log out fail.\n");
    }

__exit:
 
}
MSH_CMD_EXPORT(topctrl_log_be, Update user application firmware);
