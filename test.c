#include "uctx.h"

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

struct uctx ctx1;
struct uctx main_ctx;
unsigned long switch_cnt = 0;

// struct ucontext_t co_ctx1;
// struct ucontext_t co_main_ctx;
char stack[8192];

static void print_current_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);

    printf("%s.%03ld\n", buf, ts.tv_nsec / 1000000);
}

void func1(void *arg)
{
    double x1 = 1.111111;
    double x2 = 2.222222;
    double x3 = 3.333333;
    double x4 = 4.444444;
    double x5 = 5.555555;
    double x6 = 6.666666;
    double x7 = 7.777777;
    double x8 = 8.888888;

    for (int i = 0; i < 1000000; i++)
    {
        x1 += x2 * x3;
        x4 += x5 / x6;
        x7 += x8 - x1;
        x2 += x3 * x4;
        x6 += x7 / x8;
        swapuctx(&ctx1, &main_ctx);
    }
    double result = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
    printf("co%d result = %.15f\n", 1, result);
    swapuctx(&ctx1, &main_ctx);
}

void sighandler(int signo)
{
    printf("switch_cnt=%ld\n", switch_cnt);
    print_current_time_ms();
    exit(0);
}
int main()
{

    ctx1.stack.ss_sp = stack;
    ctx1.stack.ss_size = sizeof(stack);
    ctx1.uc_link = NULL;
    makeuctx(&ctx1, func1, NULL);

    double x1 = 1.111111;
    double x2 = 2.222222;
    double x3 = 3.333333;
    double x4 = 4.444444;
    double x5 = 5.555555;
    double x6 = 6.666666;
    double x7 = 7.777777;
    double x8 = 8.888888;

    for (int i = 0; i < 1000000; i++)
    {
        x1 += x2 * x3;
        x4 += x5 / x6;
        x7 += x8 - x1;
        x2 += x3 * x4;
        x6 += x7 / x8;
        swapuctx(&main_ctx, &ctx1);
    }

    double result = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;

    printf("co%d result = %.15f\n", 0, result);
    swapuctx(&main_ctx, &ctx1);

    printf("main func end\n");

    return 0;
}
