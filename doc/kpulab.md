
# 实验: 外部中断及外部设备的文件化
* 以kpu为例　

## 项目结构
见[doc/tree.txt](https://github.com/KeLee5453/OSCMP_10-/blob/main/doc/tree.txt)


## 相关知识
### 平台级中断控制器PLIC
PLIC(Platform-Level Interrupt Controller，平台级中断控制器)是RISCV中用来响应外部中断的一个架构定义，可以将任一外部中断源单独分配到每个CPU的外部中断上，其接受65个外部中断源作为输入。其设计逻辑如图2.4所示，并不涉及任何实际硬件的具体实现，它的寄存器块主要包括中断优先级寄存器、中断等待寄存器、中断使能寄存器、优先级阈值寄存器、中断请求寄存器、中断完成寄存器。以上寄存器都是32位的，且其按照一定的顺序进行内存分配，但中断请求寄存器与中断完成寄存器共享同一片内存。其架构如图所示：

![PLIC架构图](https://github.com/KeLee5453/OSCMP_10-/blob/main/doc/img/PLICArch.jpg)

在图中我们可以看到PLIC被分为两个部分：PLIC门和PLIC核。PLIC门作为中断源与PLIC的中继，负责接受和撤销中断源，PLIC核就用来对同时到达的中断进行仲裁，选择最佳的一个把它的中断信号和中断号发送给处理中断的目标，也就是CPU中的一个核。CPU要处理中断时就需要读取PLIC的中断请求寄存器，这个时候PLIC中对应中断源的中断等待寄存器和机器态中断等待寄存器的等待标志位都会被清零。在CPU处理完成后会回写PLIC中断完成寄存器，这个信号被传输回PLIC门，然后就可以撤销中断源的中断信号了。值得注意的一点是K210中不存在S态的外部中断，也就是操作系统本身无法捕获中断信号，并做出响应，只能依赖于运行在M态的RustSBI。

PLIC地址分布如下：
```c
/**
 * @file
 * @brief      The PLIC complies with the RISC-V Privileged Architecture
 *             specification, and can support a maximum of 1023 external
 *             interrupt sources targeting up to 15,872 core contexts.
 *
 * @note       PLIC RAM Layout
 *
 * | Address   | Description                     |
 * |-----------|---------------------------------|
 * |0x0C000000 | Reserved                        |
 * |0x0C000004 | source 1 priority               |
 * |0x0C000008 | source 2 priority               |
 * |...        | ...                             |
 * |0x0C000FFC | source 1023 priority            |
 * |           |                                 |
 * |0x0C001000 | Start of pending array          |
 * |...        | (read-only)                     |
 * |0x0C00107C | End of pending array            |
 * |0x0C001080 | Reserved                        |
 * |...        | ...                             |
 * |0x0C001FFF | Reserved                        |
 * |           |                                 |
 * |0x0C002000 | target 0 enables                |
 * |0x0C002080 | target 1 enables                |
 * |...        | ...                             |
 * |0x0C1F1F80 | target 15871 enables            |
 * |0x0C1F2000 | Reserved                        |
 * |...        | ...                             |
 * |0x0C1FFFFC | Reserved                        |
 * |           |                                 |
 * |0x0C200000 | target 0 priority threshold     |
 * |0x0C200004 | target 0 claim/complete         |
 * |...        | ...                             |
 * |0x0C201000 | target 1 priority threshold     |
 * |0x0C201004 | target 1 claim/complete         |
 * |...        | ...                             |
 * |0x0FFFF000 | target 15871 priority threshold |
 * |0x0FFFF004 | target 15871 claim/complete     |
 *
 */

```

### KPU
KPU(Knowledge Processing Unit，知识处理单元)是K210里面用来处理AI算法的单元。对于大多数应用场景来说，神经网络的运算量太大，一个实用的神经网络模型需要10层以上的计算，对于没有优化的CPU来说需要秒级甚至分钟级的时间去运算，因此非常耗时，没有实际的使用价值。神经网络的应用场景分为训练和推断，训练模型这个应用场景的工作现在已经由显卡来完成，而模型推断通常是使用人工智能技术的终端设备上，面向的是用户，对于体积和能耗都有比较高的要求，所以KPU的诞生就是为了解决模型推断运算的。

神经网路的基础运算操作包括：卷积、批归一化、激活、池化、矩阵运算。矩阵运算只有在一些新型网络结构中才会使用。K210中的KPU只支持前4种基础操作，这4种操作在KPU中并非单独的运行的，而是不可分割的，有效避免了CPU干预造成的损耗。KPU内使用的数据也很复杂，其支持16层卷积，每一层寄存器包括偏移量、图像输入输出地址、激活函数相关、常规卷积层、DMA传输参数、卷积参数、写回配置、池化参数和卷积加载参数等12个配置参数。在对KPU进行初始化的时候需要对每一层寄存器依次进行初始化：批归一表、激活表、参数加载。值得注意的一点是，输入通道的图像都是需要经过64字节宽度补齐的图像，图像宽度必须大于等于4，高度必须大于等于2。卷积参数在训练网络时使用的是浮点数进行训练，但是K210只支持int16定点数计算，所以在计算之前需要转化为定点数，主要方法就是估计动态范围进行缩放。

![KPU工作原理](https://github.com/KeLee5453/OSCMP_10-/blob/main/doc/img/kpu%E5%B7%A5%E4%BD%9C%E5%8E%9F%E7%90%86.png)

KPU进行计算的主要流程如上图所示。在计算前，先从总线上接收源数据地址、卷积核、目标数据地址、批归一化参数、池化参数和激活参数对KPU内的卷积核参数寄存器进行初始化；然后KPU就根据输入的源数据地址从通用内存中使用DMA取出数据放到KPU独享的2M内存中供计算单元使用。计算单元根据参数对源数据依次进行卷积、批归一化、激活和池化操作，然后把计算的结果写回到独享的内存中。前一层的运算结果将作为第二层运算的源数据，不断循环操作，直到最后一个卷积层运算结束，通过DMA根据输入的目标数据地址把数据写回通用内存中，再向PLIC发起中断表示自己运行结束，然后PLIC调用预设的回调函数对KPU的运算结果进行进一步处理。

### spooling技术
当我们使用打印机等低速设备的时候，有两种方式用于CPU与设备进行信息交互，一种是联机方式，一种是脱机方式。联机方式就是请求输送信息的进程需要一直等待其他进程使用外部设备资源，只能进行挂起，使得进程等待时间延长。脱机方式是通过可以被多进程共享的块设备作为CPU与外部设备的中继，进程在输出信息到块设备时，外部设备也能输入信息到块设备，提高了系统运行的并行度。但是需要人工的干预，这些设备的装卸需要人来进行。为了弥补两者的缺点，产生了spooling技术，也称假脱机技术，是一种在多道程序环境下模拟脱机方式控制外部设备进行输入输出的设备。

spooling系统分为三个模块：主控进程、输入进程、输出进程。主控进程在系统上电以后就被建立，它负责管理输入输出井的调度，并且接受来自用户态程序的请求执行响应的输入输出进程进行数据传送。输入进程就是由主控进程启动，他接收来自外部设备的数据，并把数据输送到指定的输入井中。输出进程就是把主控进程指定的输出井中的内容输出到慢速设备上。对于用户来说，通过用户界面发出请求到主控进程，选择从外部设备读取数据还是把数据输出到外部设备上。输入进程和输出进程都是主控进程的子进程，由他们来负责具体的细枝末节的工作。

而我们实现的时候，使用KPU是按照外部设备来使用的，仿照的是输入输出设备。大致的流程图如下：
![KPU spooling流程图](https://github.com/KeLee5453/OSCMP_10-/blob/main/doc/img/%E6%89%A7%E8%A1%8C%E6%B5%81.jpg)

#### 输入进程
输入进程是把用户态的数据想要送给KPU的进程，其由用户态产生，任务是把数据拷贝到内核态，把数据封装成任务，设置任务的状态，并且唤醒总控进程，由他来产生启动子进程，等待把任务送入KPU。值得注意的是我们在是实现的时候把KPU当作输入输出的2号设备，因此在用户程序时发起write系统调用时指明要写的设备是2号设备KPU。由于内核的运行地址空间与用户程序的运行地址空间不一致，所以在用户进程进入内核态后，在sysfile_write函数中把用户地址空间里的图片数据拷贝到内核空间里。然后在dev_kpu_taskinit函数中对一个全局的kpu_buff结构体实例kputaskbase进行赋值，使得其结构成员的值为用户输入的数据，这个实例在接下来就要交给总控进程进行处理，通过这样的方式进行进程间传递任务信息。在设置KPU的IO状态为初始化并且不是查询后，就调用run_kpu_task_add函数，让当前输入进程唤醒主控进程完成后续的把任务输送到KPU的工作，自己进入睡眠状态，等待总控进程的唤醒。在总控进程完成递送任务给自己的子进程后，唤醒处于睡眠状态的输入进程，输入进程逐步返回到用户态。
#### 输出进程
与输入进程类似，我们在用户态重新发起了read系统调用进入内核态来读取对应的输入进程输入到KPU的任务的运行结果，输出进程的进程号与输入进程的进程号一致。我们在写这一部分时参照的是用户态进程读取磁盘里的文件数据，内核根据读取的设备解析出传入的参数，通过file_read函数启动在内核初始化时设备预先设置的读文件函数。通过kpuio_io函数设置KPU的IO状态为查询而不是初始化，与输入进程类似，需要一个全局的结构体kpu_buff实例kpuresultbase来进行进程间通信。在dev_try_getresult函数中给kpuresultbase分配空间用来保存读取的结果。紧接着调用run_kpu_task_check函数，让当前的读取进程唤醒主控进程完成后续的读取任务，自己进入睡眠状态，等待总控进程的唤醒。在总控进程的子进程完成查询任务以后，就把结果写入kpuresultbase，其中当任务执行状态为0表示该任务运行成功，为-1表示该任务正在运行，为-2表示该任务正在等待。之后总控进程就唤醒处于睡眠状态的输出进程，输出进程逐步返回到用户态，在返回过程中要注意还需要把读取到的数据拷贝到用户空间里。这样当进程进入到用户态以后，才能正确得到想要得到的数据。
#### 总控进程
总控进程在内核进程初始化的时候被初始化为一个内核进程，当首次执行的时候，因为不满足KPU的IO条件，所以在初始化任务链表后就进入睡眠，等待输入进程与输出进程的唤醒。当输入进程唤醒总控进程时，由于KPU的IO状态已经变更，所以可以进入满足添加任务条件的分支，在里面调用add_kpu_task函数把新的任务加入到任务链表，设置任务的状态为“初始化”，并返回任务的ID。接着创建一个子进程执行try_run_task函数来监听KPU，当KPU的状态为忙时进入睡眠，当KPU执行完发起中断时候再唤醒等待着KPU运行结束的子进程。如果KPU当前的状态为空闲，则直接把任务输送给KPU的运行模块，当KPU执行完后，修改任务的状态为“运行成功”，该子进程的任务完成，返回总控进程。总控进程就把输入进程唤醒，让其完成后续工作，自己便进入睡眠状态。

当读取进程唤醒总控进程时，进入的是程序中满足读取条件的分支，就开始创建一个子进程来执行try_check_result函数，并传入读取进程的进程号。子进程接下来就在任务链表中不断查询自己的进程号对应的任务，如果查询到任务的状态为正在执行，该子进程就进入睡眠，等待KPU的中断到来；如果查询到的任务的状态为完成就直接返回到总控进程，总控进程再把读取进程唤醒，让其完成后续的工作，总控进程就进入睡眠状态，如此往复，直到输入的任务全部被执行完毕。

## 实验1  移植PLIC
想要KPU正常的工作，就需要把PLIC的较为完整的功能进行移植。首先当不同的外部中断完成时需要有不同的中断响应函数，并且KPU在运行时，向DMA发起的中断号可能是一致的，但是回调函数不一致，所以就需要这个结构来动态的注册中断处理函数。因此需要一个可以保存中断函数的结构体和中断注册函数。在启动uCoreOS时还需要对PLIC进行初始化，对我们需要使用到的中断进行使能，并设置中断优先级，我们在设置优先级时，由于我们将要设置的串口中断、DMA的中断、KPU中断在发起的时候并不会引起冲突，所以就设置其优先级都为1。

### 实现方法
具体的移植过程是，我们根据官方SDK先声明不同中断对应的中断号：33号是串口中断、32号是DMA的5号通道的中断、27号是DMA的0号通道的中断、25号是来自KPU的中断。然后定义了一个结构体`plic_instance`来存储回调函数的指针、回调函数使用的参数。还需要在内核初始化的函数里加入PLIC初始化函数，其内容是，声明一个32位的指针指向PLIC对应的中断使能寄存器，这个寄存器是对0-31号中断进行使能，然后把该寄存器中第27位和第25位置1，同理，第二个中断使能寄存器是对应32-63号中断，对其第0位和第1位置1。然后设置中断优先级寄存器，每个中断对应的中断优先级寄存器的地址就是PLIC的基地址偏移中断号乘上32位，然后在对应的地址写入1即可。
### 初始化PLIC
```c
//lab8/kern/diver/plic.c
void plic_init(void)//PLIC初始化函数
{
    //设置优先级
    writel(1, PLIC_BASE_ADDR + IRQN_UARTHS_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA0_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA5_INTERRUPT * sizeof(uint32_t));
    // writel(1, PLIC_BASE_ADDR + IRQN_AI_INTERRUPT * sizeof(uint32_t));

    for (int i = 0; i < IRQN_MAX; i++)
    {
        /* clang-format off */
        plic_instance[i] = (const plic_instance_t){
            .callback = NULL,
            .ctx      = NULL,
        };
        /* clang-format on */
    }
    cprintf("plic_init\n");
}
void plicinithart(void)
{
#ifdef QEMU
    // set uart's enable bit for this hart's S-mode.
    *(uint32 *)PLIC_SENABLE(hart) = (1 << UART_IRQ) | (1 << DISK_IRQ);
    // set this hart's S-mode priority threshold to 0.
    *(uint32 *)PLIC_SPRIORITY(hart) = 0;
#else
    //中断使能
    uint32_t *hart_m_enable = (uint32_t *)PLIC_MENABLE;
    cprintf("hart_m_enable  old:%x\n", readl(hart_m_enable));
    *(hart_m_enable) = readl(hart_m_enable) | (1 << IRQN_DMA0_INTERRUPT) | (1 << IRQN_AI_INTERRUPT);
    cprintf("hart_m_enable:%x\n", readl(hart_m_enable));
    uint32_t *hart0_m_int_enable_hi = hart_m_enable + 1;
    *(hart0_m_int_enable_hi) = readl(hart0_m_int_enable_hi) | (1 << (IRQN_UARTHS_INTERRUPT % 32)) | (1 << (IRQN_DMA5_INTERRUPT % 32));
#endif
#ifdef DEBUG
    printf("plicinithart\n");
#endif
}
```
### 中断响应函数
此外还要修改中断响应函数，单独写一个外部中断的函数，在这个函数中对捕获的外部中断进行分发，其流程与移植uCoreOS到K210上一致。首先读取PLIC的中断请求寄存器，获取引发中断的中断号，然后使用分支语句进行分发。如果是串口发起的中断就调用串口的中断处理函数，但如果是KPU和DMA5号通道的中断就需要执行保存在`plic_instance`结构中的函数，并传入对应的参数。分支结束仍然需要把中断号回写到PLIC的中断完成寄存器，PLIC才能撤销该中断。

``` c
//lab8/kern/trap/trap.c
void trap_in_ext()

{
    //读取中断请求寄存器获取中断号
    volatile uint32_t *hart0m_claim = (volatile uint32_t *)PLIC_MCLAIM;
    uint32_t irq = *hart0m_claim;
    switch (irq)//分发
    {

    case IRQN_UARTHS_INTERRUPT:
        // cprintf("[ext-trap]: %d", irq);
        dev_intr();
        break;
    case IRQN_DMA0_INTERRUPT:
        // cprintf("IRQN_DMA0_INTERRUPT");
        break;
    case IRQN_AI_INTERRUPT:
    case IRQN_DMA5_INTERRUPT: //AI
        dmac_chanel_interrupt_clear(DMAC_CHANNEL5);
        // cprintf("[ext-trap]: %d", irq);
        switch (plic_callback_flag)
        {
        case cnn_continue_flag:
            cnn_continue(plic_instance[irq].ctx);
            break;
        case cnn_input_done_flag:
            cnn_input_done(plic_instance[irq].ctx);
            break;
        case cnn_run_all_done_flag:
            cnn_run_all_done(plic_instance[irq].ctx);
            break;
        case ai_done_flag:
            ai_done(plic_instance[irq].ctx);
        default:
            break;
        }
        break;
    default:
        break;
    }
    *hart0m_claim = irq;//回写中断完成寄存器

    return;
}
```

## 实验2  移植KPU 
我们把KPU的功能模块根据K210的编程指南从一个[例子](https://github.com/sipeed/LicheeDan_K210_examples/tree/0b121730109d465ef04590b5cb2d5a905943fcf5/src/ai_demo_sim)中移植过来，迁移过来的代码主要是KPU的驱动的运行模块在`lab8/kern/driver/cnn.c`中。另外KPU在运行时需要模型参与计算，模型是由模型编译器给出，我们直接引用的例子中的已经编译好的参数。在`lab8/kern/libs/gencode_output.c`中。
### 实现方法
KPU在原SDK中使用的时候，其输入源是DVP，因此我们需要更改其输入源为用户输入，输入的是jpg图片的二进制编码，然后通过图片解码模块进行解码成RGB888格式的字符串。再把字符串输送给KPU的主体，KPU的主体再进行运算与识别，再对结果进行预测。
### 图片输入
由于在开发套件时候通常带有摄像模组，因此官方SDK里面对于KPU的使用通常是同DVP一同使用的，所以当我们想要使用KPU去识别我们自己的图片，就需要对图片预先做一些处理。经过官方文档我们得知，在摄像模组捕获的一帧一帧数据都是RGB565格式的，一个像素用16位来保存；将要输入到KPU中的图片是RGB888格式的，一个像素用24位来保存。我们平时使用的图片以JPG格式的为多数，因此我们需要把JPG格式的图片转化为一连串的无符号的8位二进制数组。在这里我们借用了[Rich Geldreich](https://github.com/richgel999/picojpeg)的图片处理单元，只需输入按照二进制格式读取的图片的编码就可以把图片处理成KPU能够识别的RGB888形式。在这个图片处理单元中，首先根据输入的编码判断是不是JPG图片，然后把图片放缩成与摄像模组捕获一样的宽为320，高为240的大小，然后就根据JPG的压缩规则对图片进行解压，最后输出长度为320x240x3=230400的无符号8位二进制数组。代码位于`lab8/kern/kpu_test/picojpeg*.[ch]`

### 主体
我们有了图片输入与KPU的驱动以后就需要进行封装，把KPU的运行过程封装成`kpu_test`函数，位于`lab8/kern/kpu_test/kpu_test.[ch]`中。主要流程是解码图片、初始化任务、进行卷积神经网络计算、进行预测。
```c
//lab8/kern/kpu_test/kpu_test.c
void kpu_test(_kpu_pool_task_t *runtask)
{

    /*---------------加载图片到ai_buf-----------------*/
    // decode jpeg
    uint32_t jpeg_size=runtask->input->jpgsize;
    cprintf("decoding jpg...\n");
    jpeg_image_t *jpeg = pico_jpeg_decode(runtask->input->jpeg, jpeg_size);
    // cprintf("decoede use :%ld us\r\n", sysctl_get_time_us() - tm);
    for (uint32_t i = 0; i < 10; i++)
    {
        /* code */ cprintf("jped->data:%d   \n", (*(jpeg->img_data + i)));
    }
    for (uint32_t i = 0; i < 230400; i++)
    {
        g_ai_buf[i] = *(jpeg->img_data + i);
    }
    cprintf("g_ai_buf addr:  %16x\n", g_ai_buf);
    /*---------------加载图片到ai_buf-----------------*/
    //存在访存问题
    cprintf("task addr : 0x%16x", task);
    //初始化任务
    cnn_task_init(&task);
    cprintf("task_init succeed\n");
    //进行卷积神经网络计算
    cnn_run(&task, 5, g_ai_buf, image_dst, ai_done_flag);

    // do_sleep(10);
    ai_done(runtask);
    while (!g_ai_done_flag)
        ;
    cprintf("cnn_run succeed\n");

    g_ai_done_flag = 0;
    //进行预测
    region_layer_cal((uint8_t *)image_dst);
    region_layer_draw_boxes(print_class);

    // return;
}

```
## 实验3　仿照dev_stdin 与dev_stdout实现 dev_kpu, 实现对应接口并在dev_init时初始化
### 实现方法
* 在`lab8\kern\fs\devs`中添加新文件dev_kpuio.c
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

## 实验4 实现kpu-spooling
### 用来接收并复原kpu-int的add_kpu_task(int callerpid)

add_kpu_task

    kpu-spooling管理器需要复原kpu-buff，并创建新的任务线程。
    由于之前在dev_kpuio的dev_kpuio_taskinit，已经将多块缓存以及缓存长度存入了bufs和lens数组中，add_kpu_task中只需要一次按长度拷贝每一块缓存，并拼接到一起即可。

    拼接后，进行总长度检查，检查是否与先前在kpu-buff中设定的长度相符合。

    接着，创建一个新的_kpu_pool_task_t，进行初始化，并加入kpu_tasklist队列。调用task_init设置状态flag，最终返回新task的id即可
```c
//lab8/kern/fs/devs/kpu_spooling.c

/**
 * add_kpu_task - add new task to kpu_tasklist
 * returns 0 if success, else -1
 * 1. copy totlength bytes from bufs to new kpu_buff
 * 2. reset kpu_buff->jpeg
 * 3. init a new task using kpu_buff
 * 4. check kpu_buff's members 
 * 5. add task to kputasklist
 * 6. set flag for the new task 
 */
int add_kpu_task(int callerpid){
    assert(totlength > 0);
    kpu_buff* buff = kmalloc(totlength);
    cprintf("[add_kpu_task]buff's totsize is %d\n", totlength);
    char* dst_ptr, *src_ptr;
    dst_ptr = (char*)buff;
    for(int i = 0; i < numblock; i++){
        char* src_ptr = (char*)bufs[i];
        cprintf("[add_kpu_task]copying %d bytes from %p to %p\n", lens[i], src_ptr, dst_ptr);
        for(int j = 0; j < lens[i]; j++){
            *dst_ptr = *src_ptr;
            dst_ptr++;
            src_ptr++;
        }
        kfree(bufs[i]);
        lens[i] = 0;
    }
    if(buff->totsize != totlength){
        panic("[add_kpu_task]tot_size doesn't matchd should be %d but %d\n", buff->totsize , totlength);
    }
    buff->jpeg = (char*) buff + (offsetof(kpu_buff, jpeg) + sizeof(uintptr_t));
    _kpu_pool_task_t* newtask;
    cprintf("[add_kpu_task], buff addr:%p,current pid:%d\n", buff,current->pid);
    if(buff->jpgsize > 0){
        newtask =  kmalloc(sizeof(_kpu_pool_task_t));
        newtask->id = alloc_id();
        if(newtask->id >= 0){
            newtask->input = buff;
            newtask->proc = find_proc(callerpid);

            list_add_before(&kpu_tasklist, &(newtask->task_link));
            cprintf("[add_kpu_task]taskid: %d, totsize %d\n",newtask->id,
            newtask->input->totsize);
            cprintf("[add_kpu_task]check jpg: ");
            for(int j = 0; j < 10; j++){
                cprintf("%d:%c ",j, newtask->input->jpeg[j]);
            }
            task_init(newtask); //set flag
            cprintf("\n");
            cprintf("[add_kpu_task]jpeg addr:%p, \n", newtask->input->jpeg);
            return newtask->id;
        }
    }
    else{
        warn("add fail 99\n");
        return 99;
    }
    return 100;
FREE_TASK:
    kfree(newtask);    
    return -1;
}
```
### 用来返回任务状态的int try_check_result(int pid)

try_check_result

    逻辑非常简单，只需要遍历kpu_tasklist ，检查是否为空后，
    遍历所有task，若找到对应进程的第一个task则返回该任务当前的状态。

    状态共有四种RESULT_GOT、RESULT_RUNNING、RESULT_WAITING、RESULT_NOTEXIST

    用户程序可以根据这个返回值判断是否继续循环等待

```c
//lab8/kern/fs/devs/kpu_spooling.c
/** 
 * try_check_result - check all kpu tasks maintained by controler
 * it does such things:
 *  1. if kpu_tasklist not empty 
 *  2. get task use le2task
 *  3. return RESULT_GOT | RESULT_RUNNING | RESULT_WAITING | RESULT_NOTEXIST
 *      according to task's status
 */
int try_check_result(int pid){
    list_entry_t* e = list_next(&kpu_tasklist);
    _kpu_pool_task_t* waitingtask = NULL;
    if (list_empty(&kpu_tasklist)){
        warn("empty list\n");
    }
    //check if its running
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(!is_running(task) && is_success(task) && pid == task->proc->pid){
            return RESULT_GOT;
        }
        else if(is_running(task)) {
            if(pid == task->proc->pid){
                return RESULT_RUNNING;
            }
        }
        else{
            if (pid == task->proc->pid){
                return RESULT_WAITING;
            }
        }
        e = list_next(e);
    }

    return RESULT_NOTEXIST;
}

```
### 用来管理所有kpuio请求的控制进程函数int kpu_task_ctrl(void *arg)

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

```c
////lab8/kern/fs/devs/kpu_spooling.c
/**
 * kernel thread used for controlling kpu tasks. 
 * when it awakes, this thread will do such things:
 * 
 * 0. call kpu_spooling_init(); then enter loop
 * loop{
 * IF kpuio_init AND !kpuio_check 
 *      GOTO ADDTASK;
 * FI
 * IF !kpuio_init AND kpuio_check 
 *      GOTO CHECKTASK;
 * FI
 *   sleep current kernel thread, so that caller proc can take over
 * }
 * 
 * ADDTASK:
 *  1. get new taskid using add_kpu_task(caller_pid)
 *  2. check if task init success; if not thrown warning 
 *  3. wake up caller proc
 * 
 * CHECKTASK:
 *  1. use try_check_result to get caller_pid's kputask 
 *  2. set kpuresultbase->result to deliver message using fs
 *  3. wake up caller proc, it will transport msg from kernel to user stack 
 */
int kpu_task_ctrl(void *arg) {
    kpu_spooling_init();
    while(1){
        bool intr_flag;
        local_intr_save(intr_flag);
        if(kpuio_init && !kpuio_check){
            //add task
            if (caller_pid > 2){
                int kputaskid = -2;        
                cprintf("[kpu_task_ctrl]add task current pid %d ,base %p\n" ,current->pid, kputaskbase);

                kputaskid = add_kpu_task(caller_pid);
                if(kputaskid < 0 || kputaskid >= 99){
                    warn("[kpu_task_ctrl]adding new task into pool fail, callerpid: %d; id %d\n", caller_pid, kputaskid);
                }else{
                    cprintf("[kpu_task_ctrl]adding new task into pool %d\n", kputaskid);
                }
                if (try_run_task(kputaskid) == -1){
                    warn("[kpu_task_ctrl]try_run_task thread init fail\n");
                }
                struct proc_struct* proc = find_proc(caller_pid);
                wakeup_proc(proc);
            } 
            current->state = PROC_SLEEPING;
            current->wait_state = WT_KPU_INIT;
             
        }
        else if(kpuio_check && !kpuio_init){
            cprintf("[kpu_task_ctrl]check task current pid %d\n" ,current->pid);
            
            int status = try_check_result(caller_pid);
            switch (status)
            {
            case RESULT_GOT:
                cprintf("[kpu_task_ctrl]kern found pid%d's task success\n", caller_pid);
                break;
            case RESULT_NOTEXIST:
                warn("[kpu_task_ctrl]kern found pid%d's task not exists", caller_pid);
                break;
            case RESULT_RUNNING:
                cprintf("[kpu_task_ctrl]kern found pid%d's task running\n", caller_pid);
                break;     
            case RESULT_WAITING:    
                cprintf("[kpu_task_ctrl]kern found pid%d's task waiting\n", caller_pid);
                break; 
            default:
                panic("[kpu_task_ctrl]unexpected return %d from try_check_result\n", status);
            };
            kpuresultbase->status = status;

            current->state = PROC_SLEEPING;
            current->wait_state = WT_KPU;
            struct proc_struct* proc = find_proc(caller_pid);
            wakeup_proc(proc);
        }
        else{
            //default sleep 
            cprintf("[kpu_task_ctrl]flags: init %d, check %d \n", kpuio_init, kpuio_check);
            current->state = PROC_SLEEPING;
            current->wait_state = WT_KPU;
        }
        local_intr_restore(intr_flag);
        schedule();  
    }
}

```

## 运行结果
### 进程调度
在实验中，我们在用户态测试程序中放入待输入的图片数组，编写好测试程序，用户程序的大致流程是，首先初始化需要传入到内核态的结构体，然后为了体现多进程，调用了fork系统调用来创建相同的进程。在每个进程中，我们首先通过write系统调用把数据写入内核态，然后再每隔200毫秒发起read系统效用来读取任务状态，直到任务完成才退出。当我们运行用户程序时，主要观察的是进程间的切换与调度是如何进行的，可以得到进程切换的过程如图所示：
![进程调度过程](https://github.com/KeLee5453/OSCMP_10-/blob/main/doc/img/%E8%BF%9B%E7%A8%8B%E8%BF%90%E8%A1%8C%E8%BF%87%E7%A8%8B.png)

### 待解决的问题

1. 外部中断问题
    我们对uCoreOS上针对KPU的运行结果与在裸机上的运行结果进行分析。在调试中我们发现uCoreOS目前还不能响应来自KPU和DMA的外部中断，我们尝试了很多种方法，但串口能够正常工作，这两个外设就不能。其他开发者在尝试解决外部中断这个问题时也比较模糊。由于时间仓促，并且这个中断对于实验来说是必须的，不然不能体现spooling技术的优越性，于是采用的是定时中断来模拟他们的中断，付出的代价就是KPU可能不能完全读取放在内存中的图片数据。
2. 内存问题
    在运行部分用户测试程序时会出现访存异常，PC不知所指，究其原因是在执行这些用户程序时分配的页面达到了1534个页面，也就是6MB，如果再分配页面就会报错。解决方法，首先只有在实现SD卡的文件系统，让SD卡与内存之间进行页面的换入换出才能解决这个问题。但由于时间关系，我们就只有通过缩减卷积的层数来减小uCoreOS的内核大小，付出的代价就是KPU不能识别出图片中的物体信息。
