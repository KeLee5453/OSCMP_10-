#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fpioa.h"

#include "kpu.h"
#include "plic.h"
#include "region_layer.h"
#include "sysctl.h"

#include "util.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

#define CLASS_NUMBER 20
INCBIN(model, "../kpu_test/yolo.kmodel");

kpu_model_context_t task;
static region_layer_t detect_rl;
volatile uint8_t g_ai_done_flag = 0;
static void ai_done(void *ctx) { g_ai_done_flag = 1; } //kpu执行完回调函数

uint32_t *g_lcd_gram0 __attribute__((aligned(64))); //显示图片
uint32_t *g_lcd_gram1 __attribute__((aligned(64))); //显示图片
uint8_t *g_ai_buf __attribute__((aligned(128)));    //读取图片

#define ANCHOR_NUM 5
#define BLACK 0x0000
float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};

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
    cprintf("The class is : %s\n", class_lable[class].str);
}

int kpu_test(void)
{
    // io_mux_init();
    // io_set_power();
    plic_init();

    cprintf("g_ai_buf:  %x\n", g_ai_buf);
    cprintf("kpu system start\n");

    uint8_t *model_data_align = model_data;
    if (kpu_load_kmodel(&task, model_data_align) != 0)
    {
        cprintf("\nmodel init error\n");
        while (1)
        {
            cprintf("kmodel is loading...\n");
        };
    }
    cprintf("kmodel  loaded...\n");
    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.7;
    detect_rl.nms_value = 0.3;
    region_layer_init(&detect_rl, 10, 7, 125, 320, 240);
    cprintf("kpu is running kmodel\n");
    for (uint8_t i = 0; i < 10; i++)
    {
        /* code */ cprintf("g_ai_buf:%d\n", (*(g_ai_buf + i)));
    }
    kpu_run_kmodel(&task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
    while (!g_ai_done_flag)
        ;
    g_ai_done_flag = 0;
    cprintf("kpu has run kmodel\n");
    float *output;
    size_t output_size;
    cprintf("kpu is getting output ...\n");
    kpu_get_output(&task, 0, (uint8_t **)&output, &output_size);
    for (size_t i = 0; i < 10; i++)
    {
        /* code */
        cprintf("kpu has got output : %f\n", *(output + i));
    }
    detect_rl.input = output;
    /* start region layer */
    cprintf("region layer is running \n");
    region_layer_run(&detect_rl, NULL);
    cprintf("region layer has run \n");

    cprintf("region layer is drawing box \n");
    region_layer_draw_boxes(&detect_rl, print_class);
    cprintf("region layer has drown box \n");

    return 0;
}