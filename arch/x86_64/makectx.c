#include "uctx.h"
#include "nostdc.h"

extern void __start_uctx(void) __attribute__((visibility("hidden")));
extern void __uctx_exit(void) __attribute__((visibility("hidden")));

int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg)
{
    unsigned long *sp = NULL;
    if (!ctx || !func)
        return -1;
    sp = ctx->stack.ss_sp + ctx->stack.ss_size;
    sp = (unsigned long *)((unsigned long)sp & (-16UL));

    *(--sp) = 0;
    /* return -> ctx_uctx */
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
