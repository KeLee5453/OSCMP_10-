#include <kpu_spooling.h>
#include <kmalloc.h>
#include <stdio.h>

static list_entry_t kpu_tasklist;
//暂时累计
int maxid;


static int alloc_id(){
    maxid = (maxid + 1) % MAX_TASKNUM;
    return maxid;
}
// add new task to list
// return taskid, if -1 failed
int add_kpu_task(kpu_buff* buff, int callerpid){
    _kpu_pool_task_t* newtask;
    cprintf("add_kpu_task, %p\n", buff);
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
            cprintf("%p, \n", newtask->input->jpeg);

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