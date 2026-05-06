#include "uctx.h"
#include "nostdc.h"

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

        After __swapuctx restore + addi sp,208:

          0(sp)  = func ptr
          8(sp)  = arg
          16(sp) = __uctx_exit (unused)
          24(sp) = ctx ptr

        ------ upper block (32B) ------

        fs11 … fs0  (12 doubles, 96B, reverse of fld order)

        ------ padding (8B, 16B align) ------

        s11 … s0, ra  (13 regs, 104B, reverse of ld order)

        ------ callee-saved (208B total) ------
        Frame total: 32 + 208 = 240B
    */

    /* upper block (32 bytes) */
    *(--sp) = (unsigned long)ctx;
    *(--sp) = (unsigned long)__uctx_exit;
    *(--sp) = (unsigned long)arg;
    *(--sp) = (unsigned long)func;

    /* FP callee-saved: fs0-fs11 (12 doubles, reverse of restore order) */
    *(--sp) = 0; /* fs11 */
    *(--sp) = 0; /* fs10 */
    *(--sp) = 0; /* fs9  */
    *(--sp) = 0; /* fs8  */
    *(--sp) = 0; /* fs7  */
    *(--sp) = 0; /* fs6  */
    *(--sp) = 0; /* fs5  */
    *(--sp) = 0; /* fs4  */
    *(--sp) = 0; /* fs3  */
    *(--sp) = 0; /* fs2  */
    *(--sp) = 0; /* fs1  */
    *(--sp) = 0; /* fs0  */

    /* padding (8 bytes, ensure 16B alignment for FP loads) */
    *(--sp) = 0;

    /* integer callee-saved (reverse of restore order): s11..s0, ra */
    *(--sp) = 0;                        /* s11 */
    *(--sp) = 0;                        /* s10 */
    *(--sp) = 0;                        /* s9  */
    *(--sp) = 0;                        /* s8  */
    *(--sp) = 0;                        /* s7  */
    *(--sp) = 0;                        /* s6  */
    *(--sp) = 0;                        /* s5  */
    *(--sp) = 0;                        /* s4  */
    *(--sp) = 0;                        /* s3  */
    *(--sp) = 0;                        /* s2  */
    *(--sp) = 0;                        /* s1  */
    *(--sp) = 0;                        /* s0  */
    *(--sp) = (unsigned long)__start_uctx; /* ra */

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
