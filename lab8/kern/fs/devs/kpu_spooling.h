#include <cnn.h>
#include <list.h>
#include <assert.h>
#include <stdio.h>
#include <proc.h>
#define MAX_TASKNUM 8

#define RESULT_GOT (0)
#define RESULT_WAITING (-2)
#define RESULT_RUNNING (-1)
#define RESULT_NOTEXIST (-3)

#define INIT_STATE_BIT      0x1
#define RUNNING_STATE_BIT   0x2
#define SUCCESS_STATE_BIT   0x8

typedef struct _kpu_pool_task{
    uint32_t sflag;
    uint32_t id;
    kpu_buff * input;  
    struct proc_struct *proc;   //the proc wait in this timer. If the expire time is end, then this proc will be scheduled
    list_entry_t task_link;    //the timer list
} _kpu_pool_task_t;

#define task_init(task)     (task)->sflag |= INIT_STATE_BIT
#define can_run(task)       (((task)->sflag & INIT_STATE_BIT) && \
                            !((task)->sflag & RUNNING_STATE_BIT) && \
                            !((task)->sflag & SUCCESS_STATE_BIT))

#define is_running(task)    ((task)->sflag & RUNNING_STATE_BIT)
#define run_task(task)      (task)->sflag |= RUNNING_STATE_BIT
#define stop_task(task)     (task)->sflag &= ~RUNNING_STATE_BIT
#define is_success(task)    ((task)->sflag & SUCCESS_STATE_BIT)
#define task_success(task)  (task)->sflag |= SUCCESS_STATE_BIT

#define le2task(le, member)            \
to_struct((le), _kpu_pool_task_t, member)

int add_kpu_task(int callerpid);
int try_check_result(int pid);
int try_run_task();

void kpu_spooling_init(void);