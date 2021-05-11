#include <defs.h>
#include <stdio.h>
#include <wait.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <dev.h>
#include <vfs.h>
#include <iobuf.h>
#include <inode.h>
#include <unistd.h>
#include <error.h>
#include <assert.h>
#include <sbi.h>
#include <io.h>
#include <trap.h>
#include <kpu.h>
#include <stdio.h>

static int
kpuio_open(struct device *dev, uint32_t open_flags){
    if (open_flags != O_RDWR)
    {
        return -E_INVAL;
    }
    return 0;
}

static int
kpuio_close(struct device *dev)
{
    return 0;
}
#include<kpu.h>
kpu_buff* kputaskbase;
int caller_pid;
int
dev_kpuio_taskinit(void *buf, size_t len, int pid){
    int ret = 0;
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        //for(int i = 0; i < len; i++)
        cprintf("dev_kpuio_taskinit current pid %d\n", current->pid);
        if(len != 1){
            panic("kpuio: task num > 1 error\n");
            return -E_INVAL;
        }
        
        kputaskbase = (kpu_buff*)(buf);
        caller_pid = pid;
        cprintf("%p, %d\n",((kpu_buff*)buf)->jpeg_data,((kpu_buff*)buf)->size );
        cprintf("load kputaskbase %x, %p, %d\n", (void*)kputaskbase, kputaskbase->jpeg_data, kputaskbase->size);
        //打印出来
    }
    run_kpu_task_add();
    local_intr_restore(intr_flag);
    return ret;
}

// current proc
extern struct proc_struct *current;
void* lastbuf = NULL;
static int
kpuio_io(struct device *dev, struct iobuf *iob, bool write)
{
    if (lastbuf != (void*)iob) lastbuf = iob;
    //else return 0; 
    if (write)
    {
        int ret;
        cprintf(" kpuio_io current pid %d\n", current->pid);
        ret = dev_kpuio_taskinit(iob->io_base, iob->io_resid, current->pid);
        return ret;
    }
    return -E_INVAL;
}

static int
kpuio_ioctl(struct device *dev, int op, void *data) {
    return -E_UNIMP;
}

static void
kpuio_device_init(struct device *dev)
{
    dev->d_blocks = 0;
    dev->d_blocksize = 1;
    dev->d_open = kpuio_open;
    dev->d_close = kpuio_close;
    dev->d_io = kpuio_io;
    dev->d_ioctl = kpuio_ioctl;
}

void dev_init_kpuio(void){
    struct inode *node;
    if ((node = dev_create_inode()) == NULL)
    {
        panic("stdin: dev_create_node.\n");
    }
    kpuio_device_init(vop_info(node, device));

    int ret;
    if((ret = vfs_add_dev("kpuio", node, 0)) != 0)
    {
        panic("kpuio: vfs_add_dev: %e.\n", ret);
    }
}