# can_ymodem

## 介绍

本软件包是连接can设备和ymodem的驱动包。

##  支持情况
| 包含设备         |    |
| ---------------- | -------- |
| **通讯接口**     |          |
| CAN              |  √       |


##  目录结构

| 名称 | 说明 |
| ---- | ---- |
| demo | 例子目录 |


##  许可证

can_ymodem package 遵循 Apache-2.0 许可，详见 `LICENSE` 文件。

## 依赖

- RT-Thread 3.0+
- RT-Thread 4.0+


## 获取软件包

使用 can_ymodem package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```shell
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] CAN_YMODEM: a device connect can & ymodem
             (6)   can pack data max size                                             
                   Version (v1.0.0)  --->
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。


## 设置参数

### 单个包的最大数据发送长度：
`RT_CAN_YMODEM_PACK_SIZE_TX`  可通过 menuconfig 中的 `(6)   can pack data max size ` 选项设置，



## API说明


1. 数据接收反馈

```C
void rt_can_ymodem_rx(struct rt_can_ymodem_device *can_ymodem, const void  *buffer, rt_size_t size)
```
can给ymodem发送数据，在can接受解析线程中调用。

|参数|注释|
|----|----|
|can_ymodem|设备|
|buffer|接收数据指针|
|size| 接收数据长度|
|返回|注释|
|----|----|
|void|没有|

2. 映射发送函数
```C
static int rt_hw_can_ymodem_tx(struct rt_can_ymodem_device *can_ymodem,
                                const void       *buffer,
                                rt_size_t         size)
```
ymodem给can发送数据，里面需要调用can设备的发送函数，这个函数将会注册到到can_ymodem设备上。

|参数|注释|
|----|----|
|can_ymodem|设备|
|buffer|发送数据指针|
|size| 发送数据长度|
|返回|注释|
|----|----|
|int|暂时没有使用，建议正常返回发送长度，不正常返回-RT_ERROR|

3. 注册can_ymodem设备

```C
rt_err_t rt_hw_can_ymodem_register(struct rt_can_ymodem_device *can_ymodem,
                                   const char              *name,
                                   rt_uint32_t              flag,
                                   void                    *data)
```

|参数|注释|
|----|----|
|can_ymodem|设备|
|name|设备名称|
|flag|设备打开标志|
|data|私有数据|
|返回|注释|
|----|----|
|RT_EOK|正常|
|-RT_ERROR|异常|



## 驱动示例

使用示例在 [driver](./demo) 下。


## 注意事项

1. 主要是用在can设备上，当然也可以用在uart设备上，但是不建议用在uart设备上。
2. 用在CAN设备上时，要特别注意设置的发送长度，不能超过8。


## 联系方式 & 感谢

* 维护：redoccheng
* 主页：<https://github.com/redocCheng/rt_can_ymodem>
* 邮箱：<619675912@qq.com>
