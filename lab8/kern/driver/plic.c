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
#include<sbi.h>

volatile plic_t *const plic = (volatile plic_t *)PLIC_BASE_ADDR;

int plic_callback_flag;
void plic_init(void)
{
    
    // writel(1, PLIC_BASE_ADDR + IRQN_AI_INTERRUPT * sizeof(uint32_t));
    int i;
    /* Disable all interrupts for the current core. */
    for(i = 0; i < ((PLIC_NUM_SOURCES + 32u) / 32u); i++)
        plic->target_enables.target[0].enable[i] = 0;

    static uint8_t s_plic_priorities_init_flag = 0;
    /* Set priorities to zero. */
    if(s_plic_priorities_init_flag == 0)
    {
        for(i = 0; i < PLIC_NUM_SOURCES; i++)
            plic->source_priorities.priority[i] = 0;
        s_plic_priorities_init_flag = 1;
    }

    /* Set the threshold to zero. */
    plic->targets.target[0].priority_threshold = 0;

    /*
     * A successful claim will also atomically clear the corresponding
     * pending bit on the interrupt source. A target can perform a claim
     * at any time, even if the EIP is not set.
     */
    i = 0;
    while(plic->targets.target[0].claim_complete > 0 && i < 100)
    {
        /* This loop will clear pending bit on the interrupt source */
        i++;
    }


    for (int i = 0; i < IRQN_MAX; i++)
    {
        /* clang-format off */
        plic_instance[i] = (const plic_instance_t){
            .callback = NULL,
            .ctx      = NULL,
        };
        /* clang-format on */
    }
    writel(1, PLIC_BASE_ADDR + IRQN_UARTHS_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA0_INTERRUPT * sizeof(uint32_t));
    writel(1, PLIC_BASE_ADDR + IRQN_DMA5_INTERRUPT * sizeof(uint32_t));
    cprintf("plic_init\n");
    //sbi_set_mie();
    sbi_enable_mie2();
}
#include<assert.h>
void plicinithart(void)
{
#ifdef QEMU
    // set uart's enable bit for this hart's S-mode.
    *(uint32 *)PLIC_SENABLE(hart) = (1 << UART_IRQ) | (1 << DISK_IRQ);
    // set this hart's S-mode priority threshold to 0.
    *(uint32 *)PLIC_SPRIORITY(hart) = 0;
#else
    uint32_t *hart_m_enable = (uint32_t *)PLIC_MENABLE;
    cprintf("hart_m_enable  old:%x\n", readl(hart_m_enable));
    *(hart_m_enable) = readl(hart_m_enable) | (1 << IRQN_DMA0_INTERRUPT) | (1 << IRQN_AI_INTERRUPT);
    cprintf("hart_m_enable:%x\n", readl(hart_m_enable));
    if((*hart_m_enable) & (1 <<IRQN_DMA0_INTERRUPT)){
        cprintf("checkIRQN_DMA0_INTERRUPT ok\n");
    }else{
        panic("IRQN_DMA0_INTERRUPT");
    }
    uint32_t *hart0_m_int_enable_hi = hart_m_enable + 1;
    *(hart0_m_int_enable_hi) = readl(hart0_m_int_enable_hi) | (1 << (IRQN_UARTHS_INTERRUPT % 32)) | (1 << (IRQN_DMA5_INTERRUPT % 32));
#endif
#ifdef DEBUG
    printf("plicinithart\n");
#endif
}

void plic_irq_register(uint32_t irq, int callback_flag, plic_irq_callback_t cb, void *ctx)
{
    cprintf("_start %s [plic] start run\n", __func__);
    /* Set user callback function */
    // plic_instance[irq].callback = callback;
    /* Assign user context */
    
    plic_instance[irq].ctx = ctx;
    cprintf("setting callback\n");
    plic_instance[irq].callback = cb;
    cprintf("setting callback\n");

    plic_callback_flag = callback_flag;
}

int plic_set_priority(uint32_t irq_number, uint32_t priority)
{
    /* Check parameters */
    if(PLIC_NUM_SOURCES < irq_number || 0 > irq_number)
        return -1;
    /* Set interrupt priority by IRQ number */
    plic->source_priorities.priority[irq_number] = priority;
    return 0;
}

int plic_irq_enable(int32_t irq_number)
{
    /* Check parameters */
    if(PLIC_NUM_SOURCES < irq_number || 0 > irq_number)
        return -1;
    //unsigned long core_id = current_coreid();
    /* Get current enable bit array by IRQ number */
    uint32_t current = plic->target_enables.target[0].enable[irq_number / 32];
    /* Set enable bit in enable bit array */
    current |= (uint32_t)1 << (irq_number % 32);
    /* Write back the enable bit array */
    plic->target_enables.target[0].enable[irq_number / 32] = current;
    return 0;
}