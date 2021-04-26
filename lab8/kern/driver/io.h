//
// Created by lumin on 2020/11/4.
//

#ifndef LAB8_IO_H
#define LAB8_IO_H

#include <defs.h>

#define readb(addr) (*(volatile uint8_t *)(addr))
#define readw(addr) (*(volatile uint16_t *)(addr))
#define readl(addr) (*(volatile uint32_t *)(addr))
#define readq(addr) (*(volatile uint64_t *)(addr))
#define writeb(v, addr)                      \
    {                                        \
        (*(volatile uint8_t *)(addr)) = (v); \
    }
#define writew(v, addr)                       \
    {                                         \
        (*(volatile uint16_t *)(addr)) = (v); \
    }
#define writel(v, addr)                       \
    {                                         \
        (*(volatile uint32_t *)(addr)) = (v); \
    }
#define writeq(v, addr)                       \
    {                                         \
        (*(volatile uint64_t *)(addr)) = (v); \
    }
/* Under Coreplex */
#define CLINT_BASE_ADDR (0x02000000U)
#define PLIC_BASE_ADDR (0x0C000000U)
#define PLIC_PRIORITY (PLIC_BASE_ADDR + 0x0)
#define PLIC_PENDING (PLIC_BASE_ADDR + 0x1000)
#define PLIC_MENABLE (PLIC_BASE_ADDR + 0x2000)
#define PLIC_SENABLE (PLIC_BASE_ADDR + 0x2080)
#define PLIC_MPRIORITY (PLIC_BASE_ADDR + 0x200000)
#define PLIC_SPRIORITY (PLIC_BASE_ADDR + 0x201000)
#define PLIC_MCLAIM (PLIC_BASE_ADDR + 0x200004)
#define PLIC_SCLAIM (PLIC_BASE_ADDR + 0x201004)

// #define UARTHS_IRQ (0x0C200004U)

/* Under TileLink */
#define UARTHS_DATA_REG (0x38000004U)
#define GPIOHS_BASE_ADDR (0x38001000U)

#define IO_BASE_ADDR (0x40000000U)
#define IO_SIZE (6 * 1024 * 1024U)

#define AI_RAM_BASE_ADDR (0x80600000U)
#define AI_RAM_SIZE (2 * 1024 * 1024U)

#define AI_IO_BASE_ADDR (0x40600000U)
#define AI_IO_SIZE (2 * 1024 * 1024U)

#define AI_BASE_ADDR (0x40800000U)
#define AI_SIZE (12 * 1024 * 1024U)

/* Under AHB 32 bit */
#define DMAC_BASE_ADDR (0x50000000U)

#define SPI_SLAVE_BASE_ADDR (0x50240000U)
#define FPIOA_BASE_ADDR (0x502B0000U)
#define SYSCTL_BASE_ADDR (0x50440000U)
#define SPI0_BASE_ADDR (0x52000000U)
#define SPI1_BASE_ADDR (0x53000000U)
#define SPI3_BASE_ADDR (0x54000000U)

#define IO_REGION_NUM (5)
//fix this to hold DMAC_BASE_ADDR & PLIC_BASE_ADDR
#define IO_REGION_START0 (0x0C000000U) //(0x0C200000U)
#define IO_REGION_END0 (0x10000000U)   //(0x0FFFF004U)//(0x0C201000U)
#define IO_REGION_START1 (0x38000000U)
#define IO_REGION_END1 (0x38002000U)

#define IO_REGION_START2 (0x40000000U) //AI
#define IO_REGION_END2 (0x44000000U)

#define IO_REGION_START3 (0x50000000U) //(0x50240000U)
#define IO_REGION_END3 (0x55000000U)
#define IO_REGION_START4 (0x80000000U)
#define IO_REGION_END4 (0x80000000U + PTSIZE)

typedef struct
{
    uintptr_t io_start;
    uintptr_t io_end;
    uintptr_t io_size;
} io_region_t;

#endif //LAB8_IO_H
