#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fpioa.h"

// #include "kpu.h"
#include "plic.h"
#include "sysctl.h"
#include <kmalloc.h>
#include "util.h"
#include "picojpeg.h"
#include "picojpeg_util.h"
#include "incbin.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "region_layer.h"
#include "cnn.h"
#include <proc.h>
#include <wait.h>
#include <sync.h>
#include <kpu_test.h>
#define CLASS_NUMBER 20
#define PLL1_OUTPUT_FREQ 400000000UL

// INCBIN(model, "../kpu_test/yolo.kmodel");
static cnn_task_t task;
static uint64_t image_dst[(10 * 7 * 125 + 7) / 8] __attribute__((aligned(128)));
static volatile uint8_t g_ai_done_flag = 0;
void ai_done(void *ctx) { g_ai_done_flag = 1; } //kpu执行完回调函数

uint8_t g_ai_buf[320 * 240 * 3] __attribute__((aligned(128)));

static wait_queue_t __wait_queue, *wait_queue = &__wait_queue;
int current_task_num;
#if (CLASS_NUMBER > 1)
typedef struct
{
    char *str;
} class_lable_t; //类的结构,可以加东西
//可以识别哪些类
class_lable_t class_lable[CLASS_NUMBER] = {
    {"airplane"}, {"bicycle"}, {"bird"}, {"boat"}, {"bottle"}, {"bus"}, {"car"}, {"cat"}, {"chair"}, {"cow"}, {"diningtable"}, {"dog"}, {"horse"}, {"motorbike"}, {"person"}, {"pottedplant"}, {"sheep"}, {"sofa"}, {"train"}, {"tvmonitor"}};

#endif

//画识别出的东西的框框
static void print_class(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                        uint32_t class, float prob)
{
    cprintf("_start %s [print_class] start run\n", __func__);

    cprintf("The class is : %s\n", class_lable[class].str);
}

int kpu_init(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    wait_queue_init(wait_queue);
    return 0;
}

void kpu_test(char jpeg_data[], uint32_t jpeg_size)
{

    /*---------------加载图片到ai_buf-----------------*/
    // decode jpeg
    cprintf("decoding jpg...\n");
    jpeg_image_t *jpeg = pico_jpeg_decode(jpeg_data, jpeg_size);
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
    cnn_task_init(&task);
    cprintf("task_init succeed\n");
    cnn_run(&task, 5, g_ai_buf, image_dst, ai_done_flag);

    do_sleep(10);
    // ai_done(NULL);
    // while (!g_ai_done_flag)
    //     ;
    cprintf("cnn_run succeed\n");

    g_ai_done_flag = 0;

    region_layer_cal((uint8_t *)image_dst);
    region_layer_draw_boxes(print_class);

    // return;
}

int kpu_request(char jpeg_data[], uint32_t jpeg_size)
{
    bool intr_flag;
    local_intr_save(intr_flag);
    if (current_task_num < TASK_NUM)
    {
        kup_buffs[current_task_num].jpg_data = jpeg_data;
        kup_buffs[current_task_num].jpg_size = jpeg_size;
        current_task_num++;
    }
    else
    {
        wait_t __wait, *wait = &__wait;
        wait_current_set(wait_queue, wait, WT_KPU);
        local_intr_restore(intr_flag);

        schedule();

        local_intr_save(intr_flag);
        wait_current_del(wait_queue, wait);
        local_intr_restore(intr_flag);
    }
}

int kpu_spooling(char jpeg_data[], uint32_t jpeg_size)
{

}