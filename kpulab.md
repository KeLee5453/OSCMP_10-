# 实验: 外部中断及外部设备的文件化
* 以kpu为例　

## 实验１　仿照dev_stdin 与dev_stdout实现 dev_kpu, 实现对应接口并在dev_init时初始化
### 实现方法
* 在lab8\kern\fs\devs中添加新文件dev_kpuio.c
* 完成函数 kpuio_open 、 kpuio_close 、 kpuio_io、  dev_init_kpuio
* 其中dev_init_kpuio进行如下操作：
  * 调用dev_create_inode为kpu创建inode。
  * 为对应的device接口接入对应的函数，以供文件系统访问
  * 调用vfs_add_dev注册名为“kpuio”的新设备

* 其中kpuio_open做如下操作：
  * 对open_flags检查，如果为可读可写则通过，否则返回错误

* 对于kpuio_close暂时不添加操作，由于正常情况下kpu管理器不会关闭，所以可以暂时不做

* kpuio_io操作比较复杂，将在后文介绍。
###  kpuio_io的实现方法
 kpuio_io应当同时支持kpu设备的输入与输出，其中输入即为设置任务的参数；输出只需支持传回一小段编码后的数据即可满足需求。

kpuio_io的输出需要进行如下操作：

* 开辟一块kpu_buff大熊啊的空间
* 试图从kpu_spooling controler得到该pid对应kpu task的信息
* 将得到的kpu_buff中的信息填入iobuf中
* 清理垃圾


 # kpuio的写入需要改


