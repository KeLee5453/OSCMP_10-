# 目前完成的工作
1. k210-example 剔除了对senor的依赖，可以从内存通过DMA读取图片，
2. KPU有输出。

# 目前存在的问题
1. 目前没有处理好的图像，不管是显示还是测试识别分类都存在问题。
2. 计划使用ucore SD卡读取照片，型号闪迪 sdxc a2 128G，购买了一个硬件debug工具 spieed USB-JTAG
3. 着手移植K210-example函数调用链上的函数，！一定要注意处理外部中断和运行态。参考[rcore](https://github.com/wyfcyx/osnotes/blob/master/rCore-Tutorial-v3/%E5%BD%BB%E5%BA%95%E8%A7%A3%E5%86%B3k210%E5%A4%96%E9%83%A8%E4%B8%AD%E6%96%AD.md)

# 已解决的问题

