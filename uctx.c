#define _GNU_SOURCE
#include "uctx.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <ucontext.h>

extern void __start_uctx(void) __attribute__((visibility("hidden")));
extern void __uctx_exit(void) __attribute__((visibility("hidden")));
extern void __swapuctx(unsigned long *, unsigned long *) __attribute__((visibility("hidden")));


__attribute__((visibility("hidden"))) void uctx_exit(struct uctx *uctx)
{
    uctx->__alive = 0;
    while (uctx->uc_link)
    {
        swapuctx(uctx, uctx->uc_link);
    }
    pthread_exit(NULL);
}

/* global functions */
int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg)
{

    unsigned long *sp = NULL;
    if (!ctx || !func)
        return -1;
    sp = ctx->stack.ss_sp + ctx->stack.ss_size;
    sp = (unsigned long *)((unsigned long)sp & (-16UL));

    *(--sp) = 0;
    /* return → ctx_uctx */
    *(--sp) = (unsigned long)ctx;
    *(--sp) = (unsigned long)__uctx_exit;

    /* parameters */
    *(--sp) = (unsigned long)arg;
    *(--sp) = (unsigned long)func;

    *(--sp) = (unsigned long)__start_uctx;

    /* callee-saved  */
    *(--sp) = 0; // rbp
    *(--sp) = 0; // rbx
    *(--sp) = 0; // r12
    *(--sp) = 0; // r13
    *(--sp) = 0; // r14
    *(--sp) = 0; // r15

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}

void swapuctx(struct uctx *octx, struct uctx *nctx)
{
    __swapuctx(&octx->sp, &nctx->sp);
}
