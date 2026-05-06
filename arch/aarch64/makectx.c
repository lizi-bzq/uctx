#include "uctx.h"
#include "nostdc.h"

extern void __start_uctx(void) __attribute__((visibility("hidden")));
extern void __uctx_exit(void) __attribute__((visibility("hidden")));

int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg)
{
    unsigned long *sp;
    if (!ctx || !func)
        return -1;

    sp = (unsigned long *)(ctx->stack.ss_sp + ctx->stack.ss_size);
    sp = (unsigned long *)((unsigned long)sp & (-16UL));

    /*
        Stack layout (from high to low):

        high addr
        +------------------+
        |    ctx ptr       |  → for __uctx_exit
        |     arg          |  → for __start_uctx
        |   func addr      |  → for __start_uctx
        |     x30          |  = __start_uctx (restored last by ldp x29, x30)
        |     x29 = 0      |
        |     x28 = 0      |
        |     x27 = 0      |
        |     x26 = 0      |
        |     x25 = 0      |
        |     x24 = 0      |
        |     x23 = 0      |
        |     x22 = 0      |
        |     x21 = 0      |
        |     x20 = 0      |
        |     x19 = 0      |  ← sp points here (restored first by ldp x19, x20)
        +------------------+
        low addr

        Total: 15 entries × 8 bytes = 120 bytes
        sp must be 16-byte aligned for ldp/stp in __swapuctx
    */

    /* Move sp down by 120 bytes (15 registers × 8 bytes) */
    sp = (unsigned long *)((unsigned long)sp - 120);

    /* Ensure 16-byte alignment */
    sp = (unsigned long *)((unsigned long)sp & (-16UL));

    /* Fill the stack frame using inline assembly to prevent compiler
       from optimizing the stores into stp with a different base address */
    __asm__ volatile(
        "stp %[arg], %[ctx], [%[sp], #104]\n\t"
        "stp %[start], %[func], [%[sp], #88]\n\t"
        "stp xzr, xzr, [%[sp], #72]\n\t"
        "stp xzr, xzr, [%[sp], #56]\n\t"
        "stp xzr, xzr, [%[sp], #40]\n\t"
        "stp xzr, xzr, [%[sp], #24]\n\t"
        "stp xzr, xzr, [%[sp], #8]\n\t"
        "str xzr, [%[sp]]\n\t"
        :
        : [sp] "r" (sp),
          [ctx] "r" ((unsigned long)ctx),
          [arg] "r" ((unsigned long)arg),
          [func] "r" ((unsigned long)func),
          [start] "r" ((unsigned long)__start_uctx)
        : "memory"
    );

    ctx->sp = (unsigned long)sp;
    ctx->__alive = 1;
    return 0;
}
