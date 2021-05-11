#include <stdio.h>
#include <ulib.h>
#define printf(...)                     fprintf(1, __VA_ARGS__)
#define kpuprintf(...)                     fprintf(2, __VA_ARGS__)
#define putc(c)                         printf("%c", c)
char ttt[5]; 

int
main(void) {
    printf("Hello world!!.\n");
    cprintf("I am process %d.\n", getpid());
    cprintf("hello pass.\n");
    for(int i = 0; i < 100; i++) ttt[i] = i+10;
    kpuprintf("%x", ttt, 1);
    return 0;
}

