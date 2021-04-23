/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#include <stddef.h>
//#include <stdint.h>
#include "defs.h"
#include "riscv.h"
#include "trap.h"
#include "plic.h"
#include "syscall.h"
#include "stdio.h"
#include "io.h"

void plic_init(void)
{
    writel(1, PLIC_BASE_ADDR + DISK_IRQ * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + UART_IRQ * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + AI_IRQ * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + DMA5_IRQ * sizeof(uint32_t));
    cprintf("plic_init\n");
}

void plicinithart(void)
{
#ifdef QEMU
    // set uart's enable bit for this hart's S-mode.
    *(uint32 *)PLIC_SENABLE(hart) = (1 << UART_IRQ) | (1 << DISK_IRQ);
    // set this hart's S-mode priority threshold to 0.
    *(uint32 *)PLIC_SPRIORITY(hart) = 0;
#else
    uint32_t *hart_m_enable = (uint32_t *)PLIC_MENABLE;
    *(hart_m_enable) = readd(hart_m_enable) | (1 << DISK_IRQ);
    uint32_t *hart0_m_int_enable_hi = hart_m_enable + 1;
    *(hart0_m_int_enable_hi) = readd(hart0_m_int_enable_hi) | (1 << (UART_IRQ % 32));
#endif
#ifdef DEBUG
    printf("plicinithart\n");
#endif
}
// ask the PLIC what interrupt we should serve.
int plic_claim(void)
{
    int irq;
#ifndef QEMU
    irq = *(uint32_t *)PLIC_MCLAIM;
#else
    irq = *(uint32_t *)PLIC_SCLAIM;
#endif
    return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq)
{
#ifndef QEMU
    *(uint32_t *)PLIC_MCLAIM = irq;
#else
    *(uint32_t *)PLIC_SCLAIM = irq;
#endif
}

void plic_set_irq(int irq)
{
    writel(1, PLIC_BASE_ADDR + irq * sizeof(uint32_t));
}

void plic_clear_irq(int irq)
{
    writel(0, PLIC_BASE_ADDR + irq * sizeof(uint32_t));
}