#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
__swapuctx:
    /*
        callee-saved registers (x19-x28, x29(fp), x30(lr))
    */
    stp x29, x30, [sp, #-16]!
    stp x27, x28, [sp, #-16]!
    stp x25, x26, [sp, #-16]!
    stp x23, x24, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    stp x19, x20, [sp, #-16]!

    /* switch stack */
    mov x9, sp
    str x9, [x0]
    ldr x9, [x1]
    mov sp, x9

    /* restore callee-saved registers */
    ldp x19, x20, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x27, x28, [sp], #16
    ldp x29, x30, [sp], #16

    ret

.global __start_uctx
.type __start_uctx, @function
__start_uctx:
    /* co-func and co-arg */
    ldp x9, x0, [sp], #16
    /* blr sets x30 to return address (next instruction) */
    blr x9

.global __uctx_exit
.type __uctx_exit, @function
__uctx_exit:
    /* the uctx */
    ldr x0, [sp], #8
    b uctx_exit
