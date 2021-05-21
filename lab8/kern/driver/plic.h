#ifndef __PLIC_H
#define __PLIC_H

#include "memlayout.h"

/**
 * @file
 * @brief      The PLIC complies with the RISC-V Privileged Architecture
 *             specification, and can support a maximum of 1023 external
 *             interrupt sources targeting up to 15,872 core contexts.
 *
 * @note       PLIC RAM Layout
 *
 * | Address   | Description                     |
 * |-----------|---------------------------------|
 * |0x0C000000 | Reserved                        |
 * |0x0C000004 | source 1 priority               |
 * |0x0C000008 | source 2 priority               |
 * |...        | ...                             |
 * |0x0C000FFC | source 1023 priority            |
 * |           |                                 |
 * |0x0C001000 | Start of pending array          |
 * |...        | (read-only)                     |
 * |0x0C00107C | End of pending array            |
 * |0x0C001080 | Reserved                        |
 * |...        | ...                             |
 * |0x0C001FFF | Reserved                        |
 * |           |                                 |
 * |0x0C002000 | target 0 enables                |
 * |0x0C002080 | target 1 enables                |
 * |...        | ...                             |
 * |0x0C1F1F80 | target 15871 enables            |
 * |0x0C1F2000 | Reserved                        |
 * |...        | ...                             |
 * |0x0C1FFFFC | Reserved                        |
 * |           |                                 |
 * |0x0C200000 | target 0 priority threshold     |
 * |0x0C200004 | target 0 claim/complete         |
 * |...        | ...                             |
 * |0x0C201000 | target 1 priority threshold     |
 * |0x0C201004 | target 1 claim/complete         |
 * |...        | ...                             |
 * |0x0FFFF000 | target 15871 priority threshold |
 * |0x0FFFF004 | target 15871 claim/complete     |
 *
 */

/**
 * @brief       PLIC External Interrupt Numbers
 *
 * @note        PLIC interrupt sources
 *
 * | Source | Name                     | Description                        |
 * |--------|--------------------------|------------------------------------|
 * | 0      | IRQN_NO_INTERRUPT        | The non-existent interrupt         |
 * | 1      | IRQN_SPI0_INTERRUPT      | SPI0 interrupt                     |
 * | 2      | IRQN_SPI1_INTERRUPT      | SPI1 interrupt                     |
 * | 3      | IRQN_SPI_SLAVE_INTERRUPT | SPI_SLAVE interrupt                |
 * | 4      | IRQN_SPI3_INTERRUPT      | SPI3 interrupt                     |
 * | 5      | IRQN_I2S0_INTERRUPT      | I2S0 interrupt                     |
 * | 6      | IRQN_I2S1_INTERRUPT      | I2S1 interrupt                     |
 * | 7      | IRQN_I2S2_INTERRUPT      | I2S2 interrupt                     |
 * | 8      | IRQN_I2C0_INTERRUPT      | I2C0 interrupt                     |
 * | 9      | IRQN_I2C1_INTERRUPT      | I2C1 interrupt                     |
 * | 10     | IRQN_I2C2_INTERRUPT      | I2C2 interrupt                     |
 * | 11     | IRQN_UART1_INTERRUPT     | UART1 interrupt                    |
 * | 12     | IRQN_UART2_INTERRUPT     | UART2 interrupt                    |
 * | 13     | IRQN_UART3_INTERRUPT     | UART3 interrupt                    |
 * | 14     | IRQN_TIMER0A_INTERRUPT   | TIMER0 channel 0 or 1 interrupt    |
 * | 15     | IRQN_TIMER0B_INTERRUPT   | TIMER0 channel 2 or 3 interrupt    |
 * | 16     | IRQN_TIMER1A_INTERRUPT   | TIMER1 channel 0 or 1 interrupt    |
 * | 17     | IRQN_TIMER1B_INTERRUPT   | TIMER1 channel 2 or 3 interrupt    |
 * | 18     | IRQN_TIMER2A_INTERRUPT   | TIMER2 channel 0 or 1 interrupt    |
 * | 19     | IRQN_TIMER2B_INTERRUPT   | TIMER2 channel 2 or 3 interrupt    |
 * | 20     | IRQN_RTC_INTERRUPT       | RTC tick and alarm interrupt       |
 * | 21     | IRQN_WDT0_INTERRUPT      | Watching dog timer0 interrupt      |
 * | 22     | IRQN_WDT1_INTERRUPT      | Watching dog timer1 interrupt      |
 * | 23     | IRQN_APB_GPIO_INTERRUPT  | APB GPIO interrupt                 |
 * | 24     | IRQN_DVP_INTERRUPT       | Digital video port interrupt       |
 * | 25     | IRQN_AI_INTERRUPT        | AI accelerator interrupt           |
 * | 26     | IRQN_FFT_INTERRUPT       | FFT accelerator interrupt          |
 * | 27     | IRQN_DMA0_INTERRUPT      | DMA channel0 interrupt             |
 * | 28     | IRQN_DMA1_INTERRUPT      | DMA channel1 interrupt             |
 * | 29     | IRQN_DMA2_INTERRUPT      | DMA channel2 interrupt             |
 * | 30     | IRQN_DMA3_INTERRUPT      | DMA channel3 interrupt             |
 * | 31     | IRQN_DMA4_INTERRUPT      | DMA channel4 interrupt             |
 * | 32     | IRQN_DMA5_INTERRUPT      | DMA channel5 interrupt             |
 * | 33     | IRQN_UARTHS_INTERRUPT    | Hi-speed UART0 interrupt           |
 * | 34     | IRQN_GPIOHS0_INTERRUPT   | Hi-speed GPIO0 interrupt           |
 * | 35     | IRQN_GPIOHS1_INTERRUPT   | Hi-speed GPIO1 interrupt           |
 * | 36     | IRQN_GPIOHS2_INTERRUPT   | Hi-speed GPIO2 interrupt           |
 * | 37     | IRQN_GPIOHS3_INTERRUPT   | Hi-speed GPIO3 interrupt           |
 * | 38     | IRQN_GPIOHS4_INTERRUPT   | Hi-speed GPIO4 interrupt           |
 * | 39     | IRQN_GPIOHS5_INTERRUPT   | Hi-speed GPIO5 interrupt           |
 * | 40     | IRQN_GPIOHS6_INTERRUPT   | Hi-speed GPIO6 interrupt           |
 * | 41     | IRQN_GPIOHS7_INTERRUPT   | Hi-speed GPIO7 interrupt           |
 * | 42     | IRQN_GPIOHS8_INTERRUPT   | Hi-speed GPIO8 interrupt           |
 * | 43     | IRQN_GPIOHS9_INTERRUPT   | Hi-speed GPIO9 interrupt           |
 * | 44     | IRQN_GPIOHS10_INTERRUPT  | Hi-speed GPIO10 interrupt          |
 * | 45     | IRQN_GPIOHS11_INTERRUPT  | Hi-speed GPIO11 interrupt          |
 * | 46     | IRQN_GPIOHS12_INTERRUPT  | Hi-speed GPIO12 interrupt          |
 * | 47     | IRQN_GPIOHS13_INTERRUPT  | Hi-speed GPIO13 interrupt          |
 * | 48     | IRQN_GPIOHS14_INTERRUPT  | Hi-speed GPIO14 interrupt          |
 * | 49     | IRQN_GPIOHS15_INTERRUPT  | Hi-speed GPIO15 interrupt          |
 * | 50     | IRQN_GPIOHS16_INTERRUPT  | Hi-speed GPIO16 interrupt          |
 * | 51     | IRQN_GPIOHS17_INTERRUPT  | Hi-speed GPIO17 interrupt          |
 * | 52     | IRQN_GPIOHS18_INTERRUPT  | Hi-speed GPIO18 interrupt          |
 * | 53     | IRQN_GPIOHS19_INTERRUPT  | Hi-speed GPIO19 interrupt          |
 * | 54     | IRQN_GPIOHS20_INTERRUPT  | Hi-speed GPIO20 interrupt          |
 * | 55     | IRQN_GPIOHS21_INTERRUPT  | Hi-speed GPIO21 interrupt          |
 * | 56     | IRQN_GPIOHS22_INTERRUPT  | Hi-speed GPIO22 interrupt          |
 * | 57     | IRQN_GPIOHS23_INTERRUPT  | Hi-speed GPIO23 interrupt          |
 * | 58     | IRQN_GPIOHS24_INTERRUPT  | Hi-speed GPIO24 interrupt          |
 * | 59     | IRQN_GPIOHS25_INTERRUPT  | Hi-speed GPIO25 interrupt          |
 * | 60     | IRQN_GPIOHS26_INTERRUPT  | Hi-speed GPIO26 interrupt          |
 * | 61     | IRQN_GPIOHS27_INTERRUPT  | Hi-speed GPIO27 interrupt          |
 * | 62     | IRQN_GPIOHS28_INTERRUPT  | Hi-speed GPIO28 interrupt          |
 * | 63     | IRQN_GPIOHS29_INTERRUPT  | Hi-speed GPIO29 interrupt          |
 * | 64     | IRQN_GPIOHS30_INTERRUPT  | Hi-speed GPIO30 interrupt          |
 * | 65     | IRQN_GPIOHS31_INTERRUPT  | Hi-speed GPIO31 interrupt          |
 *
 */

#ifdef QEMU // QEMU
#define UART_IRQ 10
#define DISK_IRQ 1
#else // k210
#define IRQN_UARTHS_INTERRUPT 33
#define IRQN_DMA5_INTERRUPT 32
#define IRQN_DMA4_INTERRUPT 31
#define IRQN_DMA3_INTERRUPT 30
#define IRQN_DMA2_INTERRUPT 29
#define IRQN_DMA1_INTERRUPT 28
#define IRQN_DMA0_INTERRUPT 27
#define IRQN_AI_INTERRUPT 25
#define IRQN_MAX 65
#define PLIC_NUM_SOURCES    (IRQN_MAX - 1)
#endif

#define cnn_input_done_flag 1
#define cnn_continue_flag 2
#define cnn_run_all_done_flag 3
#define ai_done_flag 4
// void *_task;


/**
 * @brief       Definitions for the interrupt callbacks
 */
typedef int (*plic_irq_callback_t)(void *ctx);

/**
 * @brief       Definitions for IRQ table instance
 */
typedef struct _plic_instance_t
{
    plic_irq_callback_t callback;
    void *ctx;
} plic_instance_t;
static plic_instance_t plic_instance[IRQN_MAX];

typedef struct _plic_source_priorities
{
    /* 0x0C000000: Reserved, 0x0C000004-0x0C000FFC: 1-1023 priorities */
    uint32_t priority[1024];
} __attribute__((packed, aligned(4))) plic_source_priorities_t;

/**
 * @brief       Interrupt Pending Bits
 *
 *              The current status of the interrupt source pending bits in the
 *              PLIC core can be read from the pending array, organized as 32
 *              words of 32 bits. The pending bit for interrupt ID N is stored
 *              in bit (N mod 32) of word (N/32). Bit 0 of word 0, which
 *              represents the non-existent interrupt source 0, is always
 *              hardwired to zero. The pending bits are read-only. A pending
 *              bit in the PLIC core can be cleared by setting enable bits to
 *              only enable the desired interrupt, then performing a claim. A
 *              pending bit can be set by instructing the associated gateway to
 *              send an interrupt service request.
 */
typedef struct _plic_pending_bits
{
    /* 0x0C001000-0x0C00107C: Bit 0 is zero, Bits 1-1023 is pending bits */
    uint32_t u32[32];
    /* 0x0C001080-0x0C001FFF: Reserved */
    uint8_t resv[0xF80];
} __attribute__((packed, aligned(4))) plic_pending_bits_t;

/**
 * @brief       Target Interrupt Enables
 *
 *              For each interrupt target, each device’s interrupt can be
 *              enabled by setting the corresponding bit in that target’s
 *              enables registers. The enables for a target are accessed as a
 *              contiguous array of 32×32-bit words, packed the same way as the
 *              pending bits. For each target, bit 0 of enable word 0
 *              represents the non-existent interrupt ID 0 and is hardwired to
 *              0. Unused interrupt IDs are also hardwired to zero. The enables
 *              arrays for different targets are packed contiguously in the
 *              address space. Only 32-bit word accesses are supported by the
 *              enables array in RV32 systems. Implementations can trap on
 *              accesses to enables for non-existent targets, but must allow
 *              access to the full enables array for any extant target,
 *              treating all non-existent interrupt source’s enables as
 *              hardwired to zero.
 */
typedef struct _plic_target_enables
{
    /* 0x0C002000-0x0C1F1F80: target 0-15871 enables */
    struct
    {
        uint32_t enable[32 * 2]; /* Offset 0x00-0x7C: Bit 0 is zero, Bits 1-1023 is bits*/
    } target[15872 / 2];

    /* 0x0C1F2000-0x0C1FFFFC: Reserved, size 0xE000 */
    uint8_t resv[0xE000];
} __attribute__((packed, aligned(4))) plic_target_enables_t;

/**
 * @brief       PLIC Targets
 *
 *              Target Priority Thresholds The threshold for a pending
 *              interrupt priority that can interrupt each target can be set in
 *              the target’s threshold register. The threshold is a WARL field,
 *              where different implementations can support different numbers
 *              of thresholds. The simplest implementation has a threshold
 *              hardwired to zero.
 *
 *              Target Claim Each target can perform a claim by reading the
 *              claim/complete register, which returns the ID of the highest
 *              priority pending interrupt or zero if there is no pending
 *              interrupt for the target. A successful claim will also
 *              atomically clear the corresponding pending bit on the interrupt
 *              source. A target can perform a claim at any time, even if the
 *              EIP is not set. The claim operation is not affected by the
 *              setting of the target’s priority threshold register.
 *
 *              Target Completion A target signals it has completed running a
 *              handler by writing the interrupt ID it received from the claim
 *              to the claim/complete register. This is routed to the
 *              corresponding interrupt gateway, which can now send another
 *              interrupt request to the PLIC. The PLIC does not check whether
 *              the completion ID is the same as the last claim ID for that
 *              target. If the completion ID does not match an interrupt source
 *              that is currently enabled for the target, the completion is
 *              silently ignored.
 */
typedef struct _plic_target
{
    /* 0x0C200000-0x0FFFF004: target 0-15871 */
    struct
    {
        uint32_t priority_threshold; /* Offset 0x000 */
        uint32_t claim_complete;     /* Offset 0x004 */
        uint8_t resv[0x1FF8];        /* Offset 0x008, Size 0xFF8 */
    } target[15872 / 2];
} __attribute__((packed, aligned(4))) plic_target_t;

/**
 * @brief       Platform-Level Interrupt Controller
 *
 *              PLIC is Platform-Level Interrupt Controller. The PLIC complies
 *              with the RISC-V Privileged Architecture specification, and can
 *              support a maximum of 1023 external interrupt sources targeting
 *              up to 15,872 core contexts.
 */
typedef struct _plic
{
    /* 0x0C000000-0x0C000FFC */
    plic_source_priorities_t source_priorities;
    /* 0x0C001000-0x0C001FFF */
    const plic_pending_bits_t pending_bits;
    /* 0x0C002000-0x0C1FFFFC */
    plic_target_enables_t target_enables;
    /* 0x0C200000-0x0FFFF004 */
    plic_target_t targets;
} __attribute__((packed, aligned(4))) plic_t;

extern volatile plic_t *const plic;

/**
 * @brief       Register user callback function by IRQ number
 *
 * @param[in]   irq             The irq
 * @param[in]   callback        The callback
 * @param       ctx             The context
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
void plic_irq_register(uint32_t irq,int callback_flag ,plic_irq_callback_t callback, void *ctx);

void plic_init(void);

// enable PLIC for each hart
void plicinithart(void);

// ask PLIC what interrupt we should serve
int plic_claim(void);

// tell PLIC that we've served this IRQ
void plic_complete(int irq);

void plic_set_irq(int irq);

void plic_clear_irq(int irq);

int plic_set_priority(uint32_t irq_number, uint32_t priority);
int plic_irq_enable(int32_t irq_number);

#endif
