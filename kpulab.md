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

* 开辟一块kpu_buff大小的空间
* 试图从kpu_spooling controler得到该pid对应kpu task的信息
* 将得到的kpu_buff中的信息填入iobuf中
* 清理垃圾

kpuio_io的输入需要进行如下操作：
* 使用check_magic判断这次接受的数据块是否是第一块
* 如果是，则初始化numblock， totlength， sumlen， caller_pid
* 如果不是，检查是否是同一个caller_pid, 正常执行逻辑下应当与第一块的caller_pid相同，若不同则panic
* 对于每一块，都逐Byte复制到bufs的对应位置中暂存，同时更新numblock与sumlen
* 检查sumlen是否等于totlength，若等于则使用run_kpu_task_add添加任务
* 若大于totlength则警告


## 实验2 实现kpu-spooling中，用来接收并复原kpu-int的add_kpu_task(int callerpid)

add_kpu_task

    kpu-spooling管理器需要复原kpu-buff，并创建新的任务线程。
    由于之前在dev_kpuio的dev_kpuio_taskinit，已经将多块缓存以及缓存长度存入了bufs和lens数组中，add_kpu_task中只需要一次按长度拷贝每一块缓存，并拼接到一起即可。

    拼接后，进行总长度检查，检查是否与先前在kpu-buff中设定的长度相符合。

    接着，创建一个新的_kpu_pool_task_t，进行初始化，并加入kpu_tasklist队列。调用task_init设置状态flag，最终返回新task的id即可

## 实验3 实现kpu-spooling中，用来返回任务状态的int try_check_result(int pid)

try_check_result

    逻辑非常简单，只需要遍历kpu_tasklist ，检查是否为空后，
    遍历所有task，若找到对应进程的第一个task则返回该任务当前的状态。

    状态共有四种RESULT_GOT、RESULT_RUNNING、RESULT_WAITING、RESULT_NOTEXIST

    用户程序可以根据这个返回值判断是否继续循环等待

## 实验4 实现kpu-spookling中，用来管理所有kpuio请求的控制进程函数int kpu_task_ctrl(void *arg)

int kpu_task_ctrl(void *arg)

    在调用kpu_spooling_init初始化之后，进入死循环

1.  关闭终端
2. 如果被添加任务请求唤醒（kpuio_init && !kpuio_check）
   
   * 确认caller_pid > 2
   * 调用add_kpu_task添加任务，获取新分配的任务号kputaskid
   * 检查是否为合法id（0-99）
   * 调用try_run_task创建一个控制kpu来执行计算的子线程。
   * 放弃执行权，唤醒caller_pid（通过设置current->state与 current->wait_state）
3. 如果被查询任务请求唤醒（!kpuio_init && kpuio_check）

    * 调用try_check_result返回任务状态status
    * 根据status输出不同的警告或提示
    * 设置返回的缓存块（kpuresultbase）的status成员
    * 放弃执行权，唤醒caller_pid（通过设置current->state与 current->wait_state）
4. 没有被任何上述任务唤醒，而是别的任务

     * 直接放弃执行权，还给shell线程（或其他线程）

## 实验3 修改外部中断的处理流程，采用修改自xv6-k210的rustsbi

