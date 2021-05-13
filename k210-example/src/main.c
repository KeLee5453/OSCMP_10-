#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board_config.h"
#include "fpioa.h"
#include "image.h"
#include "kpu.h"
#include "lcd.h"
#include "nt35310.h"
#include "picojpeg.h"
#include "picojpeg_util.h"
#include "plic.h"
#include "region_layer.h"
#include "sysctl.h"
#include "uarths.h"
#include "utils.h"
#include "w25qxx.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
#include "iomem.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

#define CLASS_NUMBER 20

#define LOAD_KMODEL_FROM_FLASH 0

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (1351592)
uint8_t *model_data;
#else
INCBIN(model, "../src/yolo.kmodel");

#endif

kpu_model_context_t task;
static region_layer_t detect_rl;

volatile uint8_t g_ai_done_flag = 0;

static void ai_done(void *ctx) { g_ai_done_flag = 1; }

uint32_t *g_lcd_gram0;  //显示图片
uint32_t *g_lcd_gram1;  //显示图片
uint8_t *g_ai_buf;      //读取图片

extern unsigned char jpeg_data[11485];
#define ANCHOR_NUM 5

// float g_anchor[ANCHOR_NUM * 2] = {1.08,  1.19, 3.42, 4.41,  6.63,
//                                   11.38, 9.42, 5.11, 16.62, 10.52};
static float g_anchor[ANCHOR_NUM * 2] = {1.889,    2.5245, 2.9465,   3.94056,
                                         3.99987,  5.3658, 5.155437, 6.92275,
                                         6.718375, 9.01025};
volatile uint8_t g_ram_mux = 0;

//初始化设备，使用宏定义区分board，详情修改 board_config.h
static void io_mux_init(void) {
  /* Init DVP IO map and function settings */
  fpioa_set_function(42, FUNC_CMOS_RST);
  fpioa_set_function(44, FUNC_CMOS_PWDN);
  fpioa_set_function(46, FUNC_CMOS_XCLK);
  fpioa_set_function(43, FUNC_CMOS_VSYNC);
  fpioa_set_function(45, FUNC_CMOS_HREF);
  fpioa_set_function(47, FUNC_CMOS_PCLK);
  fpioa_set_function(41, FUNC_SCCB_SCLK);
  fpioa_set_function(40, FUNC_SCCB_SDA);

  /* Init SPI IO map and function settings */
  fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
  fpioa_set_function(36, FUNC_SPI0_SS3);
  fpioa_set_function(39, FUNC_SPI0_SCLK);
  fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);

  sysctl_set_spi0_dvp_data(1);
}

static void io_set_power(void) {
  /* Set dvp and spi pin to 1.8V */
  sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
  sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

#if (CLASS_NUMBER > 1)
typedef struct {
  char *str;
  uint16_t color;
  uint16_t height;
  uint16_t width;
  uint32_t *ptr;
} class_lable_t;  //类的结构
//可以识别哪些类
class_lable_t class_lable[CLASS_NUMBER] = {
    {"aeroplane", GREEN},   {"bicycle", GREEN},     {"bird", GREEN},
    {"boat", GREEN},        {"bottle", 0xF81F},     {"bus", GREEN},
    {"car", GREEN},         {"cat", GREEN},         {"chair", 0xFD20},
    {"cow", GREEN},         {"diningtable", GREEN}, {"dog", GREEN},
    {"horse", GREEN},       {"motorbike", GREEN},   {"person", 0xF800},
    {"pottedplant", GREEN}, {"sheep", GREEN},       {"sofa", GREEN},
    {"train", GREEN},       {"tvmonitor", 0xF9B6}};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif
//对每一类进行初始化
static void lable_init(void) {
#if (CLASS_NUMBER > 1)
  uint8_t index;

  class_lable[0].height = 16;
  class_lable[0].width = 8 * strlen(class_lable[0].str);
  class_lable[0].ptr = lable_string_draw_ram;
  printf("func: lable init\n");
  for (index = 1; index < CLASS_NUMBER; index++) {
    class_lable[index].height = 16;
    class_lable[index].width = 8 * strlen(class_lable[index].str);
    class_lable[index].ptr =
        class_lable[index - 1].ptr +
        class_lable[index - 1].height * class_lable[index - 1].width / 2;
  }
#endif
}
//画识别出的东西的框框
static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                      uint32_t class, float prob) {
  printf("the class is %u32\n", class);
  if (x1 >= 320) x1 = 319;
  if (x2 >= 320) x2 = 319;
  if (y1 >= 240) y1 = 239;
  if (y2 >= 240) y2 = 239;

#if (CLASS_NUMBER > 1)
  lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
  lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width,
                   class_lable[class].height, class_lable[class].ptr);
  printf("The class is : %s\n", class_lable[class].str);
#else
  lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
#endif
}

int main(void) {
  /* Set CPU and dvp clk */
  // sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
  sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
  uarths_init();

  io_mux_init();   //初始化DVP输入输出映射和功能设置
  io_set_power();  //初始化DVP POWER
  plic_init();     //初始化为外部中断

  /* flash init */
  printf("flash init\n");
  // w25qxx_init(3, 0);
  // w25qxx_enable_quad_mode();
  g_ai_buf = (uint8_t *)iomem_malloc(320 * 240 * 3);
#if LOAD_KMODEL_FROM_FLASH
  model_data = (uint8_t *)malloc(KMODEL_SIZE + 255);
  uint8_t *model_data_align =
      (uint8_t *)(((uintptr_t)model_data + 255) & (~255));
  w25qxx_read_data(0xC00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
  uint8_t *model_data_align = model_data;
  // printf("model_data: %d", *model_data);
#endif

  lable_init();
  //初始化标签

  printf("LCD init\n");
  lcd_init();
  //初始化显示屏
#if BOARD_LICHEEDAN
  //设置LCD显示方向
  lcd_set_direction(DIR_YX_RLDU);
#else
  lcd_set_direction(DIR_YX_RLUD);
#endif
  lcd_clear(BLACK);
  lcd_draw_string(136, 70, "OSCMP__10-", WHITE);
  lcd_draw_string(104, 150, "20 class detection", WHITE);
  sleep(1);

  /* enable global interrupt */
  sysctl_enable_irq();

  // decode jpeg
  uint64_t tm = sysctl_get_time_us();
  jpeg_image_t *jpeg = pico_jpeg_decode(jpeg_data, sizeof(jpeg_data));
  printf("decoede use :%ld us\r\n", sysctl_get_time_us() - tm);

  for (uint32_t i = 0; i < 10; i++) {
    /* code */ printf("jped->data:%d   \n", (*(jpeg->img_data + i)));
  }
  // jpeg_to_ai(jpeg);
  uint32_t ii;
  for (ii = 0; *(jpeg->img_data + ii) != NULL; ii++) {
    /* code */;
  }
  printf("ima_data len:%d", ii);

  g_ai_buf = jpeg_data;
  /* system start */
  printf("system start\n");
  g_ram_mux = 0;
  /* init kpu */
  if (kpu_load_kmodel(&task, model_data_align) != 0) {
    printf("\nmodel init error\n");
    while (1) {
    };
  }
  printf("kmodel  loaded...\n");
  detect_rl.anchor_number = ANCHOR_NUM;
  detect_rl.anchor = g_anchor;
  detect_rl.threshold = 0.5;
  detect_rl.nms_value = 0.3;
  region_layer_init(&detect_rl, 10, 7, 125, 320, 240);

  printf("kpu is running kmodel\n");
  g_ai_done_flag = 0;
  kpu_run_kmodel(&task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
  while (!g_ai_done_flag)
    ;

  printf("kpu has run kmodel\n");

  float *output;
  size_t output_size;
  printf("kpu is getting output ...\n");
  kpu_get_output(&task, 0, (uint8_t **)&output, &output_size);
  for (size_t i = 0; i < 30; i++) {
    printf("kpu has got output : %f\n", *(output + i));
  }

  detect_rl.input = output;

  /* start region layer */
  printf("region layer is running \n");
  region_layer_run(&detect_rl, NULL);
  printf("region layer has run \n");

  printf("region layer is drawing box \n");
  region_layer_draw_boxes(&detect_rl, drawboxes);
  printf("region layer has drown box \n");

  /* draw boxs */
  usleep(1000);
  if (jpeg != NULL) {
    printf("Width: %d, Height: %d, Comps: %d\r\n", jpeg->width, jpeg->height,
           jpeg->comps);
    char *p = NULL;
    switch (jpeg->scan_type) {
      case PJPG_GRAYSCALE:
        p = "GRAYSCALE";
        break;
      case PJPG_YH1V1:
        p = "H1V1";
        break;
      case PJPG_YH2V1:
        p = "H2V1";
        break;
      case PJPG_YH1V2:
        p = "H1V2";
        break;
      case PJPG_YH2V2:
        p = "H2V2";
        break;
    }

    printf("Scan type: %s\r\n", p);
    jpeg_display(0, 0, jpeg);
    usleep(1000);
  }
  for (uint32_t i = 0; i < 10; i++) {
    /* code */ printf("jpeg_display:%d   \n", (*(jpeg->img_data + i)));
  }
  return 0;
}
