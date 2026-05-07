#include "nostdc.h"
#include "uctx.h"

extern void __start_uctx(void) __attribute__((visibility("hidden")));
extern void __uctx_exit(void) __attribute__((visibility("hidden")));

int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg)
{
    volatile unsigned long *sp = NULL;
    if (!ctx || !func)
        return -1;
    sp = (volatile unsigned long *)(ctx->stack.ss_sp + ctx->stack.ss_size);
    sp = (volatile unsigned long *)((unsigned long)sp & (-16UL));

    /*
        Stack layout (high addr to low addr, push order):

        __swapuctx ret enters __start_uctx.  After ret, esp at:
          0(%esp)  = func ptr       ← popped by __start_uctx
          4(%esp)  = __uctx_exit    ← func's ret address (pre-pushed)
          8(%esp)  = arg            ← callee reads via 4(%esp) after jmp
          12(%esp) = ctx ptr         ← read by __uctx_exit

        i386 cdecl calling: after a 'call', esp points to return-addr
        and arg is at 4(%esp).  By placing __uctx_exit at +4 and
        arg at +8, the callee sees arg at 4(%esp) as expected.

        Callee-saved: ebp, edi, esi, ebx (push order = pop reverse).
    */

    *(--sp) = (unsigned long)ctx;
    *(--sp) = (unsigned long)arg;
    *(--sp) = (unsigned long)__uctx_exit;
    *(--sp) = (unsigned long)func;
    *(--sp) = (unsigned long)__start_uctx; /* ret from __swapuctx */

    /* callee-saved (push order: ebp,edi,esi,ebx → reverse of pop) */
    *(--sp) = 0; /* ebp */
    *(--sp) = 0; /* edi */
    *(--sp) = 0; /* esi */
    *(--sp) = 0; /* ebx */

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
