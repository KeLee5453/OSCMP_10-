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
    writel(1, PLIC_BASE_ADDR + IRQN_UARTHS_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA0_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA5_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_AI_INTERRUPT * sizeof(uint32_t));

    for (int i = 0; i < IRQN_MAX; i++)
    {
        /* clang-format off */
        plic_instance[i] = (const plic_instance_t){
            .callback = NULL,
            .ctx      = NULL,
        };
        /* clang-format on */
    }
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
    *(hart_m_enable) = readl(hart_m_enable) | (1 << IRQN_UARTHS_INTERRUPT);
    uint32_t *hart0_m_int_enable_hi = hart_m_enable + 1;
    *(hart0_m_int_enable_hi) = readl(hart0_m_int_enable_hi) | (1 << (IRQN_DMA0_INTERRUPT % 32));
#endif
#ifdef DEBUG
    printf("plicinithart\n");
#endif
}

void plic_irq_register(uint32_t irq, plic_irq_callback_t callback, void *ctx)
{
    cprintf("_start %s [cnn] start run\n", __func__);
    /* Set user callback function */
    plic_instance[irq].callback = callback;
    /* Assign user context */
    plic_instance[irq].ctx = ctx;
}