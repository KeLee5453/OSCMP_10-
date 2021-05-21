#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <kdebug.h>
#include <picirq.h>
#include <trap.h>
#include <clock.h>
#include <intr.h>
#include <pmm.h>
#include <vmm.h>
#include <ide.h>
#include <swap.h>
#include <proc.h>
#include <kmonitor.h>
#include <fs.h>
#include <sdcard.h>
#include <dmac.h>
#include <plic.h>
#include "../kpu_test/kpu_test.h"
int kern_init(void) __attribute__((noreturn));
void grade_backtrace(void);
static void lab1_switch_test(void);
extern unsigned char jpeg_data[11485];

int kern_init(void)
{
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    // cons_init();                // init the console

    const char *message = "(THU.CST) os is loading ...";
    cprintf("%s\n\n", message);

    print_kerninfo();

    // grade_backtrace();

    pmm_init(); // init physical memory management

    sd_init(); // init SD card driver

    pic_init(); // init interrupt controller
    idt_init(); // init interrupt descriptor table

    vmm_init(); // init virtual memory management
    sched_init();
    proc_init(); // init process table

    plic_init();
    plicinithart();    
    dmac_init();

    ide_init();  // init ide devices
    swap_init(); // init swap
    fs_init();

    clock_init();  // init clock interrupt
    intr_enable(); // enable irq interrupt

    //LAB1: CAHLLENGE 1 If you try to do it, uncomment lab1_switch_test()
    // user/kernel mode switch test
    //lab1_switch_test();
    // kpu_test(jpeg_data,sizeof(jpeg_data));
    kpu_init();

    dmac_test_ch();

    cpu_idle(); // run idle process
}

void __attribute__((noinline))
grade_backtrace2(int arg0, int arg1, int arg2, int arg3)
{
    mon_backtrace(0, NULL, NULL);
}

void __attribute__((noinline))
grade_backtrace1(int arg0, int arg1)
{
    grade_backtrace2(arg0, (int64_t)&arg0, arg1, (int64_t)&arg1);
}

void __attribute__((noinline))
grade_backtrace0(int arg0, int arg1, int arg2)
{
    grade_backtrace1(arg0, arg2);
}

void grade_backtrace(void)
{
    grade_backtrace0(0, (int64_t)kern_init, 0xffff0000);
}

static void
lab1_print_cur_status(void)
{
    static int round = 0;
    round++;
}

static void
lab1_switch_to_user(void)
{
    //LAB1 CHALLENGE 1 : TODO
}

static void
lab1_switch_to_kernel(void)
{
    //LAB1 CHALLENGE 1 :  TODO
}

static void
lab1_switch_test(void)
{
    lab1_print_cur_status();
    cprintf("+++ switch to  user  mode +++\n");
    lab1_switch_to_user();
    lab1_print_cur_status();
    cprintf("+++ switch to kernel mode +++\n");
    lab1_switch_to_kernel();
    lab1_print_cur_status();
}
