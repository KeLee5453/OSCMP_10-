#include <stdio.h>
#include <ulib.h>
#include <file.h>
#define printf(...) fprintf(1, __VA_ARGS__)
#define kpuprintf(...) fprintf(2, __VA_ARGS__)
#define putc(c) printf("%c", c)

char ttt[5] = "abcd";
struct kpu_buff buff;
struct kpu_buff* buffp;
void* result;
int main(void)
{
    buff.jpeg = &ttt[0];
    buff.jpgsize = sizeof(ttt);
    buff.jpgoff =  (char*)&ttt[0] - (char*)&buff; 
    buff.totsize = sizeof(struct kpu_buff) + sizeof(char) * sizeof(ttt);
    printf("Hello world!!.\n");
    cprintf("buff.jpeg %x\n", buff.jpeg );
    cprintf("buff.size %d\n", buff.jpgoff);
    cprintf("buff %p, jpeg. %p\n",&buff,  &ttt[0]);
    buffp = &buff;
    cprintf("buffp %d\n", buffp->jpgsize);
    cprintf("hello pass.\n");

    int status = kpuprintf("%x", &buff, sizeof(buff));
    read(2, result, sizeof(uintptr_t));
    
    cprintf("I am process %d.\n", getpid());
    return 0;
}
