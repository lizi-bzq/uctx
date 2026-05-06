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
        Stack layout (from high addr to low addr, push order):

        After __swapuctx restores callee-saved via
        pop {r4-r11,lr} + vpop {d8-d15}, sp lands past the
        100B callee-saved block.  __start_uctx then reads:

          0(sp)  = func ptr       → ldr r12, [sp, #0]
          4(sp)  = arg            → ldr r0,  [sp, #4]
          8(sp)  = __uctx_exit    → (unused)
          12(sp) = ctx ptr         → ldr r0,  [sp, #12]

        ------ upper block (16B) ------

        d15 (8B) … d8 (8B)         → vpush order (highest reg first)
        lr = __start_uctx          → push order (lr first, hence at high addr)
        r11 … r4 = 0               → (r4 last, at low addr)

        ------ callee-saved block (100B) ------

        Total: 116 bytes
    */

    /* upper block (16 bytes) */
    *(--sp) = (unsigned long)ctx;           /* ctx ptr          → sp+12 */
    *(--sp) = (unsigned long)__uctx_exit;   /* (unused)         → sp+8  */
    *(--sp) = (unsigned long)arg;           /* function argument → sp+4  */
    *(--sp) = (unsigned long)func;          /* function pointer  → sp+0  */

    /*
     * VFP callee-saved: d8-d15 (8 doubles × 8B = 64B = 16 unsigned-long pushes).
     * Emulate vpush order: d15 first (highest addr), d8 last.
     */
    *(--sp) = 0; *(--sp) = 0; /* d15 */
    *(--sp) = 0; *(--sp) = 0; /* d14 */
    *(--sp) = 0; *(--sp) = 0; /* d13 */
    *(--sp) = 0; *(--sp) = 0; /* d12 */
    *(--sp) = 0; *(--sp) = 0; /* d11 */
    *(--sp) = 0; *(--sp) = 0; /* d10 */
    *(--sp) = 0; *(--sp) = 0; /* d9  */
    *(--sp) = 0; *(--sp) = 0; /* d8  */

    /*
     * Integer callee-saved: lr (__start_uctx), r11..r4 = 0.
     * Emulate push {r4-r11,lr} order: lr first, r4 last.
     */
    *(--sp) = (unsigned long)__start_uctx;  /* lr               → sp+80 */
    *(--sp) = 0; /* r11 */
    *(--sp) = 0; /* r10 */
    *(--sp) = 0; /* r9  */
    *(--sp) = 0; /* r8  */
    *(--sp) = 0; /* r7  */
    *(--sp) = 0; /* r6  */
    *(--sp) = 0; /* r5  */
    *(--sp) = 0; /* r4  */

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
