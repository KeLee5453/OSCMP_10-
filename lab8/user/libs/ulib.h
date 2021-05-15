#ifndef __USER_LIBS_ULIB_H__
#define __USER_LIBS_ULIB_H__

#include <defs.h>

void __warn(const char *file, int line, const char *fmt, ...);
void __noreturn __panic(const char *file, int line, const char *fmt, ...);

#define warn(...) \
    __warn(__FILE__, __LINE__, __VA_ARGS__)

#define panic(...) \
    __panic(__FILE__, __LINE__, __VA_ARGS__)

#define assert(x)                              \
    do                                         \
    {                                          \
        if (!(x))                              \
        {                                      \
            panic("assertion failed: %s", #x); \
        }                                      \
    } while (0)

// static_assert(x) will generate a compile-time error if 'x' is false.
#define static_assert(x) \
    switch (x)           \
    {                    \
    case 0:              \
    case (x):;           \
    }

typedef struct _kpu_buff
{
    int64_t status;
    uint64_t totsize;
    uint64_t jpgsize;
    int64_t jpgoff;
    uintptr_t no_use;
    char jpeg[4096*10];
}kpu_buff;
void __noreturn exit(int error_code);
int fork(void);
int wait(void);
int waitpid(int pid, int *store);
void yield(void);
int kill(int pid);
int getpid(void);
void print_pgdir(void);
unsigned int gettime_msec(void);
void lab6_set_priority(uint32_t priority);
int sleep(unsigned int time);
int fprintf(int fd, const char *fmt, ...);
int __exec(const char *name, const char **argv);
void kpu_run(char jpg_data[], uint32_t jpg_size);
#endif /* !__USER_LIBS_ULIB_H__ */
