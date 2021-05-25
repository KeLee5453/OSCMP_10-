
#include <kpu_spooling.h>
#include <kmalloc.h>
#include <stdio.h>
#include <wait.h>
#include <sem.h>
#include <proc.h>
list_entry_t kpu_tasklist;
//暂时累计
int maxid;
static semaphore_t kpu_sem;

/**
 * run_kpu - child thread function, created by task_ctrl_proc
 * , make sure only one child thread is running at a time using
 * semaphere
 * - set task running flag before calling kpu_run(), kpu_run() 
 * doesn't work for now, use do_sleep to replace, after returning 
 * from kpu set stop & success task. 
 */
static int run_kpu(_kpu_pool_task_t* task){
    down(&kpu_sem);

    cprintf("[thread %d]start running task %d %p\n" ,current->pid, task->id, task);
    run_task(task);

    do_sleep(1000);

    cprintf("[run_kputhread %d], set flag success & stop\n", current->pid);
    stop_task(task);
    task_success(task);
    cprintf("[run_kputhread %d]return from run kpu, run success\n",current->pid);

    up(&kpu_sem);
}

// add_runtest_thread create a new child thread using task
int add_runtest_thread(_kpu_pool_task_t* task){
    int pid = kernel_thread(run_kpu, (void*)task, 0);
    if(pid > 2) {
        cprintf("[add_runtest_thread]created kpu thread pid %d\n", pid);
        return 0;
    }
    return -1;
}

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

/**
 * try to run a new task
 * return -1 if nothing is ready for run
 * return 0 if kpu is busy
 * return pid if kpu starts to run pid's task
 */
extern void kpu_test(_kpu_pool_task_t *runtask);
int try_run_task(int taskid){
    list_entry_t* e = list_next(&kpu_tasklist);
    _kpu_pool_task_t* runtask = NULL;
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(task->id == taskid){
            runtask = task;
            if(is_running(task)) {
                warn("trying to run a running task %p\n", task);
                break;
            }else{
                break;
            }
        }
        e = list_next(e);
    } 
    if(runtask == NULL) return -1;

    //runtask
    cprintf("[try_run_task]deliver task from pid%d, jpeg %p, size %d to kpu\n " 
            ,runtask->proc->pid, runtask->input->jpeg, runtask->input->jpgsize);
    add_runtest_thread(runtask);
    // 执行kpu_test(runtask);
    // kpu_test(runtask);
    return runtask->proc->pid;
}

//allocate a new task id
static int alloc_id(){
    maxid = (maxid + 1) % MAX_TASKNUM;
    return maxid;
}


extern int totlength;
extern void* bufs[64];
extern int lens[64];
extern int numblock;

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
        newtask =  kmalloc(buff->totsize);
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

//  kpu_spooling_init - init kpu_tasklist & kpu_sem
void kpu_spooling_init(void){
    maxid = 0;
    list_init(&kpu_tasklist);
    sem_init(&kpu_sem,1);
}


extern kpu_buff* kputaskbase, *kpuresultbase;
extern int caller_pid;
extern bool kpuio_init;
extern bool kpuio_check;
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