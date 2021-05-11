#include <kpu.h>
#include <list.h>
#include <assert.h>
#include <stdio.h>
#include <proc.h>
#define MAX_TASKNUM 8
typedef struct _kpu_pool_task{
    uint32_t id;
    kpu_buff * input;  
    struct proc_struct *proc;   //the proc wait in this timer. If the expire time is end, then this proc will be scheduled
    list_entry_t task_link;    //the timer list
} _kpu_pool_task_t;

#define le2timer(le, member)            \
to_struct((le), _kpu_pool_task_t, member)

int add_kpu_task(kpu_buff* buff, int callerpid);

void kpu_spooling_init(void);