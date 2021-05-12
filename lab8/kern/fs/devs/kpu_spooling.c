#include <kpu_spooling.h>
#include <kmalloc.h>
#include <stdio.h>

list_entry_t kpu_tasklist;
//暂时累计
int maxid;


#include <kpu_test.h>

int try_check_result(int pid){
    list_entry_t* e = list_next(&kpu_tasklist);
    _kpu_pool_task_t* waitingtask = NULL;
    if (list_empty(&kpu_tasklist)){
        warn("empty list\n");
    }
    //check if its running
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(!is_running(task) && is_success(task)){
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
int try_run_task(){
    list_entry_t* e = list_next(&kpu_tasklist);
    bool busy = false;
    _kpu_pool_task_t* runtask = NULL;
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(is_running(task)) {
            busy = true;    
            break;
        }else{
            if (runtask == NULL && can_run(task)) runtask = task;
        }
        e = list_next(e);
    } 
    if(busy) return 0;
    if(runtask == NULL) return -1;

    //runtask
    cprintf("deliver task from pid%d, jpeg %p, size %d to kpu\n " 
            ,runtask->proc->pid, runtask->input->jpeg, runtask->input->jpgsize);
    run_task(runtask);
    //执行kpu_test(runtask)
    //kpu_test();
    return runtask->proc->pid;
}
static int alloc_id(){
    maxid = (maxid + 1) % MAX_TASKNUM;
    return maxid;
}
// add new task to list
// return taskid, if -1 failed
int add_kpu_task(kpu_buff* buff, int callerpid){
    _kpu_pool_task_t* newtask;
    //cprintf("add_kpu_task, %p\n", buff);
    if(buff->jpgsize > 0){
        newtask =  kmalloc(buff->totsize);
        newtask->id = alloc_id();
        if(newtask->id >= 0){
            newtask->input = buff;
            newtask->proc = find_proc(callerpid);

            list_add_before(&kpu_tasklist, &(newtask->task_link));
            cprintf("task %d, totsize %d\n",newtask->id,
            newtask->input->totsize);
            for(int j = 0; j < newtask->input->jpgsize; j++){
                cprintf("%c ", newtask->input->jpeg[j]);
            }
            task_init(newtask); //set flag
            return newtask->id;
        }
    }
    else{
        return 99;
    }
    return 100;
FREE_TASK:
    kfree(newtask);    
    return -1;
}
 
void kpu_spooling_init(void){
    maxid = 0;
    list_init(&kpu_tasklist);

    cprintf("kpu spooling init\n");
}