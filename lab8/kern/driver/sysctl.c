//
// Created by lumin on 2020/11/2.
//
#include <sysctl.h>
#include <defs.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define LOG(fmt, args...) cprintf(fmt, ##args)

const uint8_t source_select_pll2[] =
    {
        [0] = SYSCTL_CLOCK_SOURCE_IN0,
        [1] = SYSCTL_CLOCK_SOURCE_PLL0,
        [2] = SYSCTL_CLOCK_SOURCE_PLL1};
const uint8_t get_select_pll2[] =
    {
        [SYSCTL_CLOCK_SOURCE_IN0] = 0,
        [SYSCTL_CLOCK_SOURCE_PLL0] = 1,
        [SYSCTL_CLOCK_SOURCE_PLL1] = 2,
};
volatile sysctl_t *const sysctl = (volatile sysctl_t *)SYSCTL_BASE_ADDR;

static int sysctl_clock_bus_end_en(sysctl_clock_t clock, int en);
static int sysctl_clock_device_end_en(sysctl_clock_t clock, int en);
static uint32_t sysctl_clock_source_get_freq(sysctl_clock_source_t input);

static int sysctl_clock_bus_end_en(sysctl_clock_t clock, int en)
{
    /*
     * The timer is under APB0, to prevent apb0_clk_en1 and apb0_clk_en0
     * on same register, we split it to peripheral and central two
     * registers, to protect CPU close apb0 clock accidentally.
     *
     * The apb0_clk_en0 and apb0_clk_en1 have same function,
     * one of them set, the APB0 clock enable.
     */

    /* The APB clock should carefully disable */
    if (en)
    {
        switch (clock)
        {
        /*
             * These peripheral devices are under APB0
             * GPIO, UART1, UART2, UART3, SPI_SLAVE, I2S0, I2S1,
             * I2S2, I2C0, I2C1, I2C2, FPIOA, SHA256, TIMER0,
             * TIMER1, TIMER2
             */
        case SYSCTL_CLOCK_GPIO:
        case SYSCTL_CLOCK_SPI2:
        case SYSCTL_CLOCK_I2S0:
        case SYSCTL_CLOCK_I2S1:
        case SYSCTL_CLOCK_I2S2:
        case SYSCTL_CLOCK_I2C0:
        case SYSCTL_CLOCK_I2C1:
        case SYSCTL_CLOCK_I2C2:
        case SYSCTL_CLOCK_UART1:
        case SYSCTL_CLOCK_UART2:
        case SYSCTL_CLOCK_UART3:
        case SYSCTL_CLOCK_FPIOA:
        case SYSCTL_CLOCK_TIMER0:
        case SYSCTL_CLOCK_TIMER1:
        case SYSCTL_CLOCK_TIMER2:
        case SYSCTL_CLOCK_SHA:
            sysctl->clk_en_cent.apb0_clk_en = en;
            break;

        /*
             * These peripheral devices are under APB1
             * WDT, AES, OTP, DVP, SYSCTL
             */
        case SYSCTL_CLOCK_AES:
        case SYSCTL_CLOCK_WDT0:
        case SYSCTL_CLOCK_WDT1:
        case SYSCTL_CLOCK_OTP:
        case SYSCTL_CLOCK_RTC:
            sysctl->clk_en_cent.apb1_clk_en = en;
            break;

        /*
             * These peripheral devices are under APB2
             * SPI0, SPI1
             */
        case SYSCTL_CLOCK_SPI0:
        case SYSCTL_CLOCK_SPI1:
            sysctl->clk_en_cent.apb2_clk_en = en;
            break;

        default:
            break;
        }
    }

    return 0;
}

static int sysctl_clock_device_end_en(sysctl_clock_t clock, int en)
{
    switch (clock)
    {
    /*
         * These devices are PLL
         */
    case SYSCTL_CLOCK_PLL0:
        sysctl->pll0.pll_out_en0 = en;
        break;
    case SYSCTL_CLOCK_PLL1:
        sysctl->pll1.pll_out_en1 = en;
        break;
    case SYSCTL_CLOCK_PLL2:
        sysctl->pll2.pll_out_en2 = en;
        break;

        /*
             * These devices are CPU, SRAM, APB bus, ROM, DMA, AI
             */
    case SYSCTL_CLOCK_CPU:
        sysctl->clk_en_cent.cpu_clk_en = en;
        break;
    case SYSCTL_CLOCK_SRAM0:
        sysctl->clk_en_cent.sram0_clk_en = en;
        break;
    case SYSCTL_CLOCK_SRAM1:
        sysctl->clk_en_cent.sram1_clk_en = en;
        break;
    case SYSCTL_CLOCK_APB0:
        sysctl->clk_en_cent.apb0_clk_en = en;
        break;
    case SYSCTL_CLOCK_APB1:
        sysctl->clk_en_cent.apb1_clk_en = en;
        break;
    case SYSCTL_CLOCK_APB2:
        sysctl->clk_en_cent.apb2_clk_en = en;
        break;
    case SYSCTL_CLOCK_ROM:
        sysctl->clk_en_peri.rom_clk_en = en;
        break;
    case SYSCTL_CLOCK_DMA:
        sysctl->clk_en_peri.dma_clk_en = en;
        break;
    case SYSCTL_CLOCK_AI:
        sysctl->clk_en_peri.ai_clk_en = en;
        break;
    case SYSCTL_CLOCK_DVP:
        sysctl->clk_en_peri.dvp_clk_en = en;
        break;
    case SYSCTL_CLOCK_FFT:
        sysctl->clk_en_peri.fft_clk_en = en;
        break;
    case SYSCTL_CLOCK_SPI3:
        sysctl->clk_en_peri.spi3_clk_en = en;
        break;

        /*
             * These peripheral devices are under APB0
             * GPIO, UART1, UART2, UART3, SPI_SLAVE, I2S0, I2S1,
             * I2S2, I2C0, I2C1, I2C2, FPIOA, SHA256, TIMER0,
             * TIMER1, TIMER2
             */
    case SYSCTL_CLOCK_GPIO:
        sysctl->clk_en_peri.gpio_clk_en = en;
        break;
    case SYSCTL_CLOCK_SPI2:
        sysctl->clk_en_peri.spi2_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2S0:
        sysctl->clk_en_peri.i2s0_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2S1:
        sysctl->clk_en_peri.i2s1_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2S2:
        sysctl->clk_en_peri.i2s2_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2C0:
        sysctl->clk_en_peri.i2c0_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2C1:
        sysctl->clk_en_peri.i2c1_clk_en = en;
        break;
    case SYSCTL_CLOCK_I2C2:
        sysctl->clk_en_peri.i2c2_clk_en = en;
        break;
    case SYSCTL_CLOCK_UART1:
        sysctl->clk_en_peri.uart1_clk_en = en;
        break;
    case SYSCTL_CLOCK_UART2:
        sysctl->clk_en_peri.uart2_clk_en = en;
        break;
    case SYSCTL_CLOCK_UART3:
        sysctl->clk_en_peri.uart3_clk_en = en;
        break;
    case SYSCTL_CLOCK_FPIOA:
        sysctl->clk_en_peri.fpioa_clk_en = en;
        break;
    case SYSCTL_CLOCK_TIMER0:
        sysctl->clk_en_peri.timer0_clk_en = en;
        break;
    case SYSCTL_CLOCK_TIMER1:
        sysctl->clk_en_peri.timer1_clk_en = en;
        break;
    case SYSCTL_CLOCK_TIMER2:
        sysctl->clk_en_peri.timer2_clk_en = en;
        break;
    case SYSCTL_CLOCK_SHA:
        sysctl->clk_en_peri.sha_clk_en = en;
        break;

        /*
             * These peripheral devices are under APB1
             * WDT, AES, OTP, DVP, SYSCTL
             */
    case SYSCTL_CLOCK_AES:
        sysctl->clk_en_peri.aes_clk_en = en;
        break;
    case SYSCTL_CLOCK_WDT0:
        sysctl->clk_en_peri.wdt0_clk_en = en;
        break;
    case SYSCTL_CLOCK_WDT1:
        sysctl->clk_en_peri.wdt1_clk_en = en;
        break;
    case SYSCTL_CLOCK_OTP:
        sysctl->clk_en_peri.otp_clk_en = en;
        break;
    case SYSCTL_CLOCK_RTC:
        sysctl->clk_en_peri.rtc_clk_en = en;
        break;

        /*
             * These peripheral devices are under APB2
             * SPI0, SPI1
             */
    case SYSCTL_CLOCK_SPI0:
        sysctl->clk_en_peri.spi0_clk_en = en;
        break;
    case SYSCTL_CLOCK_SPI1:
        sysctl->clk_en_peri.spi1_clk_en = en;
        break;

    default:
        break;
    }

    return 0;
}

int sysctl_clock_enable(sysctl_clock_t clock)
{
    if (clock >= SYSCTL_CLOCK_MAX) //SYSCTL_CLOCK_MAX为最大的时钟中断号
        return -1;
    sysctl_clock_bus_end_en(clock, 1); //设置APB clock，使能外围设备对应的central clock
    sysctl_clock_device_end_en(clock, 1);
    return 0;
}

int sysctl_clock_set_threshold(sysctl_threshold_t which, int threshold)
{
    int result = 0;
    switch (which)
    {
    /*
         * These threshold is 2 bit width
         */
    case SYSCTL_THRESHOLD_ACLK:
        sysctl->clk_sel0.aclk_divider_sel = (uint8_t)threshold & 0x03;
        break;

        /*
             * These threshold is 3 bit width
             */
    case SYSCTL_THRESHOLD_APB0:
        sysctl->clk_sel0.apb0_clk_sel = (uint8_t)threshold & 0x07;
        break;
    case SYSCTL_THRESHOLD_APB1:
        sysctl->clk_sel0.apb1_clk_sel = (uint8_t)threshold & 0x07;
        break;
    case SYSCTL_THRESHOLD_APB2:
        sysctl->clk_sel0.apb2_clk_sel = (uint8_t)threshold & 0x07;
        break;

        /*
             * These threshold is 4 bit width
             */
    case SYSCTL_THRESHOLD_SRAM0:
        sysctl->clk_th0.sram0_gclk_threshold = (uint8_t)threshold & 0x0F;
        break;
    case SYSCTL_THRESHOLD_SRAM1:
        sysctl->clk_th0.sram1_gclk_threshold = (uint8_t)threshold & 0x0F;
        break;
    case SYSCTL_THRESHOLD_AI:
        sysctl->clk_th0.ai_gclk_threshold = (uint8_t)threshold & 0x0F;
        break;
    case SYSCTL_THRESHOLD_DVP:
        sysctl->clk_th0.dvp_gclk_threshold = (uint8_t)threshold & 0x0F;
        break;
    case SYSCTL_THRESHOLD_ROM:
        sysctl->clk_th0.rom_gclk_threshold = (uint8_t)threshold & 0x0F;
        break;

        /*
             * These threshold is 8 bit width
             */
    case SYSCTL_THRESHOLD_SPI0:
        sysctl->clk_th1.spi0_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_SPI1:
        sysctl->clk_th1.spi1_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_SPI2:
        sysctl->clk_th1.spi2_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_SPI3:
        sysctl->clk_th1.spi3_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER0:
        sysctl->clk_th2.timer0_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER1:
        sysctl->clk_th2.timer1_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER2:
        sysctl->clk_th2.timer2_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2S0_M:
        sysctl->clk_th4.i2s0_mclk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2S1_M:
        sysctl->clk_th4.i2s1_mclk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2S2_M:
        sysctl->clk_th5.i2s2_mclk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2C0:
        sysctl->clk_th5.i2c0_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2C1:
        sysctl->clk_th5.i2c1_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2C2:
        sysctl->clk_th5.i2c2_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_WDT0:
        sysctl->clk_th6.wdt0_clk_threshold = (uint8_t)threshold;
        break;
    case SYSCTL_THRESHOLD_WDT1:
        sysctl->clk_th6.wdt1_clk_threshold = (uint8_t)threshold;
        break;

        /*
             * These threshold is 16 bit width
             */
    case SYSCTL_THRESHOLD_I2S0:
        sysctl->clk_th3.i2s0_clk_threshold = (uint16_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2S1:
        sysctl->clk_th3.i2s1_clk_threshold = (uint16_t)threshold;
        break;
    case SYSCTL_THRESHOLD_I2S2:
        sysctl->clk_th4.i2s2_clk_threshold = (uint16_t)threshold;
        break;

    default:
        result = -1;
        break;
    }
    return result;
}

int sysctl_clock_get_threshold(sysctl_threshold_t which)
{
    int threshold = 0;

    switch (which)
    {
    /*
         * Select and get threshold value
         */
    case SYSCTL_THRESHOLD_ACLK:
        threshold = (int)sysctl->clk_sel0.aclk_divider_sel;
        break;
    case SYSCTL_THRESHOLD_APB0:
        threshold = (int)sysctl->clk_sel0.apb0_clk_sel;
        break;
    case SYSCTL_THRESHOLD_APB1:
        threshold = (int)sysctl->clk_sel0.apb1_clk_sel;
        break;
    case SYSCTL_THRESHOLD_APB2:
        threshold = (int)sysctl->clk_sel0.apb2_clk_sel;
        break;
    case SYSCTL_THRESHOLD_SRAM0:
        threshold = (int)sysctl->clk_th0.sram0_gclk_threshold;
        break;
    case SYSCTL_THRESHOLD_SRAM1:
        threshold = (int)sysctl->clk_th0.sram1_gclk_threshold;
        break;
    case SYSCTL_THRESHOLD_AI:
        threshold = (int)sysctl->clk_th0.ai_gclk_threshold;
        break;
    case SYSCTL_THRESHOLD_DVP:
        threshold = (int)sysctl->clk_th0.dvp_gclk_threshold;
        break;
    case SYSCTL_THRESHOLD_ROM:
        threshold = (int)sysctl->clk_th0.rom_gclk_threshold;
        break;
    case SYSCTL_THRESHOLD_SPI0:
        threshold = (int)sysctl->clk_th1.spi0_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_SPI1:
        threshold = (int)sysctl->clk_th1.spi1_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_SPI2:
        threshold = (int)sysctl->clk_th1.spi2_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_SPI3:
        threshold = (int)sysctl->clk_th1.spi3_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER0:
        threshold = (int)sysctl->clk_th2.timer0_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER1:
        threshold = (int)sysctl->clk_th2.timer1_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_TIMER2:
        threshold = (int)sysctl->clk_th2.timer2_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S0:
        threshold = (int)sysctl->clk_th3.i2s0_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S1:
        threshold = (int)sysctl->clk_th3.i2s1_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S2:
        threshold = (int)sysctl->clk_th4.i2s2_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S0_M:
        threshold = (int)sysctl->clk_th4.i2s0_mclk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S1_M:
        threshold = (int)sysctl->clk_th4.i2s1_mclk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2S2_M:
        threshold = (int)sysctl->clk_th5.i2s2_mclk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2C0:
        threshold = (int)sysctl->clk_th5.i2c0_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2C1:
        threshold = (int)sysctl->clk_th5.i2c1_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_I2C2:
        threshold = (int)sysctl->clk_th5.i2c2_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_WDT0:
        threshold = (int)sysctl->clk_th6.wdt0_clk_threshold;
        break;
    case SYSCTL_THRESHOLD_WDT1:
        threshold = (int)sysctl->clk_th6.wdt1_clk_threshold;
        break;

    default:
        break;
    }

    return threshold;
}

int sysctl_clock_set_clock_select(sysctl_clock_select_t which, int select)
{
    int result = 0;
    switch (which)
    {
    /*
         * These clock select is 1 bit width
         */
    case SYSCTL_CLOCK_SELECT_PLL0_BYPASS:
        sysctl->pll0.pll_bypass0 = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_PLL1_BYPASS:
        sysctl->pll1.pll_bypass1 = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_PLL2_BYPASS:
        sysctl->pll2.pll_bypass2 = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_ACLK:
        sysctl->clk_sel0.aclk_sel = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_SPI3:
        sysctl->clk_sel0.spi3_clk_sel = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER0:
        sysctl->clk_sel0.timer0_clk_sel = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER1:
        sysctl->clk_sel0.timer1_clk_sel = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER2:
        sysctl->clk_sel0.timer2_clk_sel = select & 0x01;
        break;
    case SYSCTL_CLOCK_SELECT_SPI3_SAMPLE:
        sysctl->clk_sel1.spi3_sample_clk_sel = select & 0x01;
        break;

        /*
             * These clock select is 2 bit width
             */
    case SYSCTL_CLOCK_SELECT_PLL2:
        sysctl->pll2.pll_ckin_sel2 = select & 0x03;
        break;

    default:
        result = -1;
        break;
    }

    return result;
}

int sysctl_clock_get_clock_select(sysctl_clock_select_t which)
{
    int clock_select = 0;

    switch (which)
    {
    /*
         * Select and get clock select value
         */
    case SYSCTL_CLOCK_SELECT_PLL0_BYPASS:
        clock_select = (int)sysctl->pll0.pll_bypass0;
        break;
    case SYSCTL_CLOCK_SELECT_PLL1_BYPASS:
        clock_select = (int)sysctl->pll1.pll_bypass1;
        break;
    case SYSCTL_CLOCK_SELECT_PLL2_BYPASS:
        clock_select = (int)sysctl->pll2.pll_bypass2;
        break;
    case SYSCTL_CLOCK_SELECT_PLL2:
        clock_select = (int)sysctl->pll2.pll_ckin_sel2;
        break;
    case SYSCTL_CLOCK_SELECT_ACLK:
        clock_select = (int)sysctl->clk_sel0.aclk_sel;
        break;
    case SYSCTL_CLOCK_SELECT_SPI3:
        clock_select = (int)sysctl->clk_sel0.spi3_clk_sel;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER0:
        clock_select = (int)sysctl->clk_sel0.timer0_clk_sel;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER1:
        clock_select = (int)sysctl->clk_sel0.timer1_clk_sel;
        break;
    case SYSCTL_CLOCK_SELECT_TIMER2:
        clock_select = (int)sysctl->clk_sel0.timer2_clk_sel;
        break;
    case SYSCTL_CLOCK_SELECT_SPI3_SAMPLE:
        clock_select = (int)sysctl->clk_sel1.spi3_sample_clk_sel;
        break;

    default:
        break;
    }

    return clock_select;
}

static uint32_t sysctl_clock_source_get_freq(sysctl_clock_source_t input)
{
    LOG("_start %s [sysctl] start run\n", __func__);

    uint32_t result;

    switch (input)
    {
    case SYSCTL_CLOCK_SOURCE_IN0:
        result = SYSCTL_CLOCK_FREQ_IN;
        break;
    case SYSCTL_CLOCK_SOURCE_PLL0:
        result = sysctl_pll_get_freq(SYSCTL_PLL0);
        break;
    case SYSCTL_CLOCK_SOURCE_PLL1:
        result = sysctl_pll_get_freq(SYSCTL_PLL1);
        break;
    case SYSCTL_CLOCK_SOURCE_PLL2:
        result = sysctl_pll_get_freq(SYSCTL_PLL2);
        break;
    case SYSCTL_CLOCK_SOURCE_ACLK:
        result = sysctl_clock_get_freq(SYSCTL_CLOCK_ACLK);
        break;
    default:
        result = 0;
        break;
    }
    LOG("_end %s [sysctl] end run\n", __func__);

    return result;
}

uint32_t sysctl_pll_get_freq(sysctl_pll_t pll)
{
    uint32_t freq_in = 0, freq_out = 0;
    uint32_t nr = 0, nf = 0, od = 0;
    uint8_t select = 0;

    if (pll >= SYSCTL_PLL_MAX)
        return 0;

    switch (pll)
    {
    case SYSCTL_PLL0:
        freq_in = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        nr = sysctl->pll0.clkr0 + 1;
        nf = sysctl->pll0.clkf0 + 1;
        od = sysctl->pll0.clkod0 + 1;
        break;
    case SYSCTL_PLL1:
        freq_in = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        nr = sysctl->pll1.clkr1 + 1;
        nf = sysctl->pll1.clkf1 + 1;
        od = sysctl->pll1.clkod1 + 1;
        break;
    case SYSCTL_PLL2:
        /*
             * Get input freq accroding select register
             */
        select = sysctl->pll2.pll_ckin_sel2;
        if (select < sizeof(source_select_pll2))
            freq_in = sysctl_clock_source_get_freq(source_select_pll2[select]);
        else
            freq_in = 0;
        nr = sysctl->pll2.clkr2 + 1;
        nf = sysctl->pll2.clkf2 + 1;
        od = sysctl->pll2.clkod2 + 1;
        break;
    default:
        break;
    }

    freq_out = (double)freq_in / (double)nr * (double)nf / (double)od;
    return freq_out;
}

uint32_t sysctl_clock_get_freq(sysctl_clock_t clock)
{
    uint32_t source = 0;
    uint32_t result = 0;

    switch (clock)
    {
    /*
         * The clock IN0
         */
    case SYSCTL_CLOCK_IN0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        result = source;
        break;

        /*
             * These clock directly under PLL clock domain
             * They are using gated divider.
             */
    case SYSCTL_CLOCK_PLL0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source;
        break;
    case SYSCTL_CLOCK_PLL1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL1);
        result = source;
        break;
    case SYSCTL_CLOCK_PLL2:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL2);
        result = source;
        break;

        /*
             * These clock directly under ACLK clock domain
             */
    case SYSCTL_CLOCK_CPU:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_ACLK))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0) /
                     (2ULL << sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ACLK));
            break;
        default:
            break;
        }
        result = source;
        break;
    case SYSCTL_CLOCK_DMA:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_ACLK))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0) /
                     (2ULL << sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ACLK));
            break;
        default:
            break;
        }
        result = source;
        break;
    case SYSCTL_CLOCK_FFT:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_ACLK))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0) /
                     (2ULL << sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ACLK));
            break;
        default:
            break;
        }
        result = source;
        break;
    case SYSCTL_CLOCK_ACLK:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_ACLK))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0) /
                     (2ULL << sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ACLK));
            break;
        default:
            break;
        }
        result = source;
        break;
    case SYSCTL_CLOCK_HCLK:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_ACLK))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0) /
                     (2ULL << sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ACLK));
            break;
        default:
            break;
        }
        result = source;
        break;

        /*
             * These clock under ACLK clock domain.
             * They are using gated divider.
             */
    case SYSCTL_CLOCK_SRAM0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SRAM0) + 1);
        break;
    case SYSCTL_CLOCK_SRAM1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SRAM1) + 1);
        break;
    case SYSCTL_CLOCK_ROM:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_ROM) + 1);
        break;
    case SYSCTL_CLOCK_DVP:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_DVP) + 1);
        break;

        /*
             * These clock under ACLK clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_APB0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_APB0) + 1);
        break;
    case SYSCTL_CLOCK_APB1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_APB1) + 1);
        break;
    case SYSCTL_CLOCK_APB2:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_ACLK);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_APB2) + 1);
        break;

        /*
             * These clock under AI clock domain.
             * They are using gated divider.
             */
    case SYSCTL_CLOCK_AI:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL1);
        result = source / (sysctl_clock_get_threshold(SYSCTL_THRESHOLD_AI) + 1);
        break;

        /*
             * These clock under I2S clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_I2S0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL2);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2S0) + 1) * 2);
        break;
    case SYSCTL_CLOCK_I2S1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL2);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2S1) + 1) * 2);
        break;
    case SYSCTL_CLOCK_I2S2:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL2);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2S2) + 1) * 2);
        break;

        /*
             * These clock under WDT clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_WDT0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_WDT0) + 1) * 2);
        break;
    case SYSCTL_CLOCK_WDT1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_WDT1) + 1) * 2);
        break;

        /*
             * These clock under PLL0 clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_SPI0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SPI0) + 1) * 2);
        break;
    case SYSCTL_CLOCK_SPI1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SPI1) + 1) * 2);
        break;
    case SYSCTL_CLOCK_SPI2:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SPI2) + 1) * 2);
        break;
    case SYSCTL_CLOCK_I2C0:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2C0) + 1) * 2);
        break;
    case SYSCTL_CLOCK_I2C1:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2C1) + 1) * 2);
        break;
    case SYSCTL_CLOCK_I2C2:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_I2C2) + 1) * 2);
        break;

        /*
             * These clock under PLL0_SEL clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_SPI3:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_SPI3))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
            break;
        default:
            break;
        }

        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_SPI3) + 1) * 2);
        break;
    case SYSCTL_CLOCK_TIMER0:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_TIMER0))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
            break;
        default:
            break;
        }

        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_TIMER0) + 1) * 2);
        break;
    case SYSCTL_CLOCK_TIMER1:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_TIMER1))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
            break;
        default:
            break;
        }

        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_TIMER1) + 1) * 2);
        break;
    case SYSCTL_CLOCK_TIMER2:
        switch (sysctl_clock_get_clock_select(SYSCTL_CLOCK_SELECT_TIMER2))
        {
        case 0:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
            break;
        case 1:
            source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_PLL0);
            break;
        default:
            break;
        }

        result = source / ((sysctl_clock_get_threshold(SYSCTL_THRESHOLD_TIMER2) + 1) * 2);
        break;

        /*
             * These clock under MISC clock domain.
             * They are using even divider.
             */

        /*
             * These clock under APB0 clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_GPIO:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;
    case SYSCTL_CLOCK_UART1:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;
    case SYSCTL_CLOCK_UART2:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;
    case SYSCTL_CLOCK_UART3:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;
    case SYSCTL_CLOCK_FPIOA:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;
    case SYSCTL_CLOCK_SHA:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
        result = source;
        break;

        /*
             * These clock under APB1 clock domain.
             * They are using even divider.
             */
    case SYSCTL_CLOCK_AES:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB1);
        result = source;
        break;
    case SYSCTL_CLOCK_OTP:
        source = sysctl_clock_get_freq(SYSCTL_CLOCK_APB1);
        result = source;
        break;
    case SYSCTL_CLOCK_RTC:
        source = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        result = source;
        break;

        /*
             * These clock under APB2 clock domain.
             * They are using even divider.
             */
        /*
             * Do nothing.
             */
    default:
        break;
    }
    return result;
}

int sysctl_dma_select(sysctl_dma_channel_t channel, sysctl_dma_select_t select)
{
    sysctl_dma_sel0_t dma_sel0;
    sysctl_dma_sel1_t dma_sel1;

    /* Read register from bus */
    dma_sel0 = sysctl->dma_sel0;
    dma_sel1 = sysctl->dma_sel1;
    switch (channel)
    {
    case SYSCTL_DMA_CHANNEL_0:
        dma_sel0.dma_sel0 = select;
        break;

    case SYSCTL_DMA_CHANNEL_1:
        dma_sel0.dma_sel1 = select;
        break;

    case SYSCTL_DMA_CHANNEL_2:
        dma_sel0.dma_sel2 = select;
        break;

    case SYSCTL_DMA_CHANNEL_3:
        dma_sel0.dma_sel3 = select;
        break;

    case SYSCTL_DMA_CHANNEL_4:
        dma_sel0.dma_sel4 = select;
        break;

    case SYSCTL_DMA_CHANNEL_5:
        dma_sel1.dma_sel5 = select;
        break;

    default:
        return -1;
    }

    /* Write register back to bus */
    sysctl->dma_sel0 = dma_sel0;
    sysctl->dma_sel1 = dma_sel1;

    return 0;
}
static uint32_t sysctl_pll_source_set_freq(sysctl_pll_t pll, sysctl_clock_source_t source, uint32_t freq)
{
    LOG("_start %s [sysctl] start run\n", __func__);

    uint32_t freq_in = 0;

    if (pll >= SYSCTL_PLL_MAX)
        return 0;

    if (source >= SYSCTL_CLOCK_SOURCE_MAX)
        return 0;

    switch (pll)
    {
    case SYSCTL_PLL0:
    case SYSCTL_PLL1:
        /*
             * Check input clock source
             */
        if (source != SYSCTL_CLOCK_SOURCE_IN0)
            return 0;
        freq_in = sysctl_clock_source_get_freq(SYSCTL_CLOCK_SOURCE_IN0);
        LOG("SYSCTL_CLOCK_SOURCE_IN0:%d\n", freq_in);

        /*
             * Check input clock freq
             */
        if (freq_in == 0)
            return 0;
        break;

    case SYSCTL_PLL2:
        /*
             * Check input clock source
             */
        if (source < sizeof(get_select_pll2))
            freq_in = sysctl_clock_source_get_freq(source);
        /*
             * Check input clock freq
             */
        if (freq_in == 0)
            return 0;
        break;

    default:
        return 0;
    }

    /*
     * Begin calculate PLL registers' value
     */
    LOG("Begin calculate PLL registers' value\n");
    /* constants */
    const double vco_min = 3.5e+08;
    const double vco_max = 1.75e+09;
    const double ref_min = 1.36719e+07;
    const double ref_max = 1.75e+09;
    const int nr_min = 1;
    const int nr_max = 16;
    const int nf_min = 1;
    const int nf_max = 64;
    const int no_min = 1;
    const int no_max = 16;
    const int nb_min = 1;
    const int nb_max = 64;
    const int max_vco = 1;
    const int ref_rng = 1;

    /* variables */
    int nr = 0;
    int nrx = 0;
    int nf = 0;
    int nfi = 0;
    int no = 0;
    int noe = 0;
    int not = 0;
    int nor = 0;
    int nore = 0;
    int nb = 0;
    int first = 0;
    int firstx = 0;
    int found = 0;

    long long nfx = 0;
    double fin = 0, fout = 0, fvco = 0;
    double val = 0, nval = 0, err = 0, merr = 0, terr = 0;
    int x_nrx = 0, x_no = 0, x_nb = 0;
    long long x_nfx = 0;
    double x_fvco = 0, x_err = 0;
    LOG("end variables\n");
    fin = freq_in;
    fout = freq;
    val = fout / fin;
    terr = 0.5 / ((double)(nf_max / 2));
    // LOG("terr:%f\n", terr);
    first = firstx = 1;
    if (terr != -2)
    {
        first = 0;
        if (terr == 0)
            terr = 1e-16;
        // LOG("end if\n");

        merr = fabs(terr);
        // LOG("end fabs\n");
    }
    found = 0;
    for (nfi = val; nfi < nf_max; ++nfi)
    {
        nr = roundf(((double)nfi) / val);
        if (nr == 0)
            continue;
        if ((ref_rng) && (nr < nr_min))
            continue;
        if (fin / ((double)nr) > ref_max)
            continue;
        nrx = nr;
        nf = nfx = nfi;
        nval = ((double)nfx) / ((double)nr);
        if (nf == 0)
            nf = 1;
        err = 1 - nval / val;

        if ((first) || (fabs(err) < merr * (1 + 1e-6)) || (fabs(err) < 1e-16))
        {
            not = floor(vco_max / fout);
            for (no = (not > no_max) ? no_max : not ; no > no_min; --no)
            {
                if ((ref_rng) && ((nr / no) < nr_min))
                    continue;
                if ((nr % no) == 0)
                    break;
            }
            if ((nr % no) != 0)
                continue;
            nor = ((not > no_max) ? no_max : not ) / no;
            nore = nf_max / nf;
            if (nor > nore)
                nor = nore;
            noe = ceil(vco_min / fout);
            if (!max_vco)
            {
                nore = (noe - 1) / no + 1;
                nor = nore;
                not = 0; /* force next if to fail */
            }
            if ((((no * nor) < (not >> 1)) || ((no * nor) < noe)) && ((no * nor) < (nf_max / nf)))
            {
                no = nf_max / nf;
                if (no > no_max)
                    no = no_max;
                if (no > not )
                    no = not ;
                nfx *= no;
                nf *= no;
                if ((no > 1) && (!firstx))
                    continue;
                /* wait for larger nf in later iterations */
            }
            else
            {
                nrx /= no;
                nfx *= nor;
                nf *= nor;
                no *= nor;
                if (no > no_max)
                    continue;
                if ((nor > 1) && (!firstx))
                    continue;
                /* wait for larger nf in later iterations */
            }

            nb = nfx;
            if (nb < nb_min)
                nb = nb_min;
            if (nb > nb_max)
                continue;

            fvco = fin / ((double)nrx) * ((double)nfx);
            if (fvco < vco_min)
                continue;
            if (fvco > vco_max)
                continue;
            if (nf < nf_min)
                continue;
            if ((ref_rng) && (fin / ((double)nrx) < ref_min))
                continue;
            if ((ref_rng) && (nrx > nr_max))
                continue;
            if (!(((firstx) && (terr < 0)) || (fabs(err) < merr * (1 - 1e-6)) || ((max_vco) && (no > x_no))))
                continue;
            if ((!firstx) && (terr >= 0) && (nrx > x_nrx))
                continue;

            found = 1;
            x_no = no;
            x_nrx = nrx;
            x_nfx = nfx;
            x_nb = nb;
            x_fvco = fvco;
            x_err = err;
            first = firstx = 0;
            merr = fabs(err);
            if (terr != -1)
                continue;
        }
    }
    LOG("end FOR\n");
    cprintf("found:%d", found);
    if (!found)
    {
        return 0;
    }

    nrx = x_nrx;
    nfx = x_nfx;
    no = x_no;
    nb = x_nb;
    fvco = x_fvco;
    err = x_err;
    // cprintf("(terr != -2) && (fabs(err) >= terr * (1 - 1e-6)):%d", (terr != -2) && (fabs(err) >= terr * (1 - 1e-6)));
    if ((terr != -2) && (fabs(err) >= terr * (1 - 1e-6)))
    {
        return 0;
    }

    /*
     * Begin write PLL registers' value,
     * Using atomic write method.
     */
    sysctl_pll0_t pll0;
    sysctl_pll1_t pll1;
    sysctl_pll2_t pll2;
    LOG("Begin write PLL registers' value \n");
    switch (pll)
    {
    case SYSCTL_PLL0:
        /* Read register from bus */
        pll0 = sysctl->pll0;
        /* Set register temporary value */
        pll0.clkr0 = nrx - 1;
        pll0.clkf0 = nfx - 1;
        pll0.clkod0 = no - 1;
        pll0.bwadj0 = nb - 1;
        /* Write register back to bus */
        sysctl->pll0 = pll0;
        break;

    case SYSCTL_PLL1:
        LOG("_start write PLL1\n");
        /* Read register from bus */
        pll1 = sysctl->pll1;
        /* Set register temporary value */
        pll1.clkr1 = nrx - 1;
        pll1.clkf1 = nfx - 1;
        pll1.clkod1 = no - 1;
        pll1.bwadj1 = nb - 1;
        /* Write register back to bus */
        sysctl->pll1 = pll1;
        LOG("_end write PLL1\n");

        break;

    case SYSCTL_PLL2:
        /* Read register from bus */
        pll2 = sysctl->pll2;
        /* Set register temporary value */
        if (source < sizeof(get_select_pll2))
            pll2.pll_ckin_sel2 = get_select_pll2[source];

        pll2.clkr2 = nrx - 1;
        pll2.clkf2 = nfx - 1;
        pll2.clkod2 = no - 1;
        pll2.bwadj2 = nb - 1;
        /* Write register back to bus */
        sysctl->pll2 = pll2;
        break;

    default:
        return 0;
    }
    LOG("_end %s [sysctl] end run\n", __func__);

    return sysctl_pll_get_freq(pll);
}

static int sysctl_pll_is_lock(sysctl_pll_t pll)
{
    /*
     * All bit enable means PLL lock
     *
     * struct pll_lock_t
     * {
     *         uint8_t overflow : 1;
     *         uint8_t rfslip : 1;
     *         uint8_t fbslip : 1;
     * };
     *
     */

    if (pll >= SYSCTL_PLL_MAX)
        return 0;

    switch (pll)
    {
    case SYSCTL_PLL0:
        return sysctl->pll_lock.pll_lock0 == 3;

    case SYSCTL_PLL1:
        return sysctl->pll_lock.pll_lock1 & 1;

    case SYSCTL_PLL2:
        return sysctl->pll_lock.pll_lock2 & 1;

    default:
        break;
    }

    return 0;
}

static int sysctl_pll_clear_slip(sysctl_pll_t pll)
{
    if (pll >= SYSCTL_PLL_MAX)
        return -1;

    switch (pll)
    {
    case SYSCTL_PLL0:
        sysctl->pll_lock.pll_slip_clear0 = 1;
        break;

    case SYSCTL_PLL1:
        sysctl->pll_lock.pll_slip_clear1 = 1;
        break;

    case SYSCTL_PLL2:
        sysctl->pll_lock.pll_slip_clear2 = 1;
        break;

    default:
        break;
    }

    return sysctl_pll_is_lock(pll) ? 0 : -1;
}

uint32_t sysctl_pll_set_freq(sysctl_pll_t pll, uint32_t pll_freq)
{
    LOG("_start %s [sysctl] start run\n", __func__);

    if (pll_freq == 0)
        return 0;

    volatile sysctl_general_pll_t *v_pll_t;
    switch (pll)
    {
    case SYSCTL_PLL0:
        v_pll_t = (sysctl_general_pll_t *)(&sysctl->pll0);
        break;
    case SYSCTL_PLL1:
        v_pll_t = (sysctl_general_pll_t *)(&sysctl->pll1);
        break;
    case SYSCTL_PLL2:
        v_pll_t = (sysctl_general_pll_t *)(&sysctl->pll2);
        break;
    default:
        return 0;
        break;
    }
    /* 1. Change CPU CLK to XTAL */
    if (pll == SYSCTL_PLL0)
        sysctl_clock_set_clock_select(SYSCTL_CLOCK_SELECT_ACLK, SYSCTL_CLOCK_SOURCE_IN0);

    /* 2. Disable PLL output */
    v_pll_t->pll_out_en = 0;

    /* 3. Turn off PLL */
    v_pll_t->pll_pwrd = 0;

    /* 4. Set PLL new value */
    uint32_t result;
    if (pll == SYSCTL_PLL2)
        result = sysctl_pll_source_set_freq(pll, v_pll_t->pll_ckin_sel, pll_freq);
    else
        result = sysctl_pll_source_set_freq(pll, SYSCTL_CLOCK_SOURCE_IN0, pll_freq);

    /* 5. Power on PLL */
    v_pll_t->pll_pwrd = 1;
    /* wait >100ns */
    int i = 0;
    LOG("_start in while\n");
    while (i < 10000000)
    {
        i++;
    }

    /* 6. Reset PLL then Release Reset*/
    v_pll_t->pll_reset = 0;
    v_pll_t->pll_reset = 1;
    /* wait >100ns */
    i = 0;
    while (i < 100000000)
    {
        i++;
    }
    v_pll_t->pll_reset = 0;

    /* 7. Get lock status, wait PLL stable */
    while (sysctl_pll_is_lock(pll) == 0)
        sysctl_pll_clear_slip(pll);

    /* 8. Enable PLL output */
    v_pll_t->pll_out_en = 1;

    /* 9. Change CPU CLK to PLL */
    if (pll == SYSCTL_PLL0)
    {
        sysctl_clock_set_clock_select(SYSCTL_CLOCK_SELECT_ACLK, SYSCTL_CLOCK_SOURCE_PLL0);
        // uart_debug_init(-1);
    }
    return result;
}
