#include <stdio.h>
#include <ulib.h>
#define printf(...)                     fprintf(1, __VA_ARGS__)
#define kpuprintf(...)                     fprintf(2, __VA_ARGS__)
#define putc(c)                         printf("%c", c)
char ttt[5]; 
struct kpu_buff{
    uint64_t size;
    char* jpeg;
}buff;
int
main(void) {
    buff.jpeg = &ttt[0];
    buff.size = 5;
    printf("Hello world!!.\n");
    cprintf("buff.jpeg %p\n", buff.jpeg);
    cprintf("buff.size %d\n", buff.size);
    cprintf("buff. %p\n", &buff);

    cprintf("hello pass.\n");
    for(int i = 0; i < 100; i++) ttt[i] = i+10;
    int status = kpuprintf("%x", &buff, 1);
    cprintf("I am process %d.\n", getpid());
    return 0;
}

