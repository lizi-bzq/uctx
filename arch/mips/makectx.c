#include "uctx.h"
#include <stddef.h>

extern void __start_uctx(void) __attribute__((visibility("hidden")));
extern void __uctx_exit(void) __attribute__((visibility("hidden")));

int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg)
{
    unsigned long *sp = NULL;
    if (!ctx || !func)
        return -1;
    sp = ctx->stack.ss_sp + ctx->stack.ss_size;
    sp = (unsigned long *)((unsigned long)sp & (-8UL));

    /*
        Stack layout (from high to low address):
        [ctx ptr]      <- uctx pointer (for __uctx_exit)
        [__uctx_exit]  <- return address after func returns
        [arg]          <- function argument ($a0)
        [func ptr]     <- function pointer
        [__start_uctx] <- return address (for __swapuctx's jr $ra)
        [$s7]          <- callee-saved
        [$s6]
        [$s5]
        [$s4]
        [$s3]
        [$s2]
        [$s1]
        [$s0]
        [$fp]          <- 0
        [$ra]          <- 0
    */

    *(--sp) = (unsigned long)ctx;          /* ctx ptr */
    *(--sp) = (unsigned long)__uctx_exit;  /* return address after func */
    *(--sp) = (unsigned long)arg;          /* function argument */
    *(--sp) = (unsigned long)func;         /* function pointer */
    *(--sp) = (unsigned long)__start_uctx; /* return address */

    /* callee-saved registers */
    *(--sp) = 0; /* s7 */
    *(--sp) = 0; /* s6 */
    *(--sp) = 0; /* s5 */
    *(--sp) = 0; /* s4 */
    *(--sp) = 0; /* s3 */
    *(--sp) = 0; /* s2 */
    *(--sp) = 0; /* s1 */
    *(--sp) = 0; /* s0 */
    *(--sp) = 0; /* fp */
    *(--sp) = 0; /* ra */

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
