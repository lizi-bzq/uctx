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
    sp = (volatile unsigned long *)((unsigned long)sp & (-8UL));

    /*
        Stack layout (from high addr to low addr, i.e., push order):

        After __swapuctx restores callee-saved regs and
        addiu $sp, $sp, 88, sp points past the 88B block.

          0($sp)  = func ptr       → lw $t9, 0($sp)
          4($sp)  = arg            → lw $a0, 4($sp)
          8($sp)  = __uctx_exit    → (unused)
          12($sp) = ctx ptr         → lw $a0, 12($sp)

        ------ upper block (16B) ------

        f30 (8B) … f20 (8B)         → sdc1/ldc1 order (6 × 8B = 48B)

        ------ integer callee-saved ------ (save order: s0..s7,fp,ra)
        sp+36: ra = __start_uctx
        sp+32: fp = 0
        sp+28: s7 = 0
        sp+24: s6 = 0
        sp+20: s5 = 0
        sp+16: s4 = 0
        sp+12: s3 = 0
        sp+8:  s2 = 0
        sp+4:  s1 = 0
        sp+0:  s0 = 0       ← ctx->sp points here

        Total callee-saved: 88B.  Full frame: 16 + 88 = 104B.
    */

    /* upper block (16 bytes) */
    *(--sp) = (unsigned long)ctx;           /* ctx ptr          → sp+100 */
    *(--sp) = (unsigned long)__uctx_exit;   /* (unused)         → sp+96 */
    *(--sp) = (unsigned long)arg;           /* function argument → sp+92 */
    *(--sp) = (unsigned long)func;          /* function pointer  → sp+88 */

    /* FP callee-saved: $f20-$f30 (6 doubles, 48B = 12 unsigned-long pushes) */
    *(--sp) = 0; *(--sp) = 0; /* f30 → sp+80 */
    *(--sp) = 0; *(--sp) = 0; /* f28 → sp+72 */
    *(--sp) = 0; *(--sp) = 0; /* f26 → sp+64 */
    *(--sp) = 0; *(--sp) = 0; /* f24 → sp+56 */
    *(--sp) = 0; *(--sp) = 0; /* f22 → sp+48 */
    *(--sp) = 0; *(--sp) = 0; /* f20 → sp+40 */

    /*
     * Integer callee-saved (reverse of entry.S save order):
     * ra, fp, s7..s0.
     * __swapuctx will lw s0..s7,fp,ra from sp+0..sp+36
     * then jr $ra enters __start_uctx.
     */
    *(--sp) = (unsigned long)__start_uctx;  /* ra               → sp+36 */
    *(--sp) = 0; /* fp                      → sp+32 */
    *(--sp) = 0; /* s7                      → sp+28 */
    *(--sp) = 0; /* s6                      → sp+24 */
    *(--sp) = 0; /* s5                      → sp+20 */
    *(--sp) = 0; /* s4                      → sp+16 */
    *(--sp) = 0; /* s3                      → sp+12 */
    *(--sp) = 0; /* s2                      → sp+8 */
    *(--sp) = 0; /* s1                      → sp+4 */
    *(--sp) = 0; /* s0                      → sp+0 */

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
