#define TASK_NUM 20
typedef struct kpu_buff
{
    char *jpg_data;
    uint32_t jpg_size;
};
struct kpu_buff kup_buffs[TASK_NUM];
int kpu_test(char jpg_data[], uint32_t jpg_size);
int kpu_init(void);