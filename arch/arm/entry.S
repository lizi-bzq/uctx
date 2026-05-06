#include "asm-offset.h"

#ifndef __ARM_PCS_VFP
.fpu vfpv3
#endif

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, %function
__swapuctx:
    /*
        callee-saved registers:
        integer: r4-r11, lr = 9 × 4B = 36B
        fp:      d8-d15 = 8 × 8B = 64B
        total: 100B

        Save VFP first (sp is 8-byte aligned at AAPCS entry),
        then integer — avoids alignment gaps.
        Restore reverses: integer first, then VFP.
    */
    vpush {d8-d15}
    push {r4-r11, lr}

    /* switch stack */
    str sp, [r0]            /* *octx = sp */
    ldr sp, [r1]            /* sp = *nctx */

    /* restore callee-saved registers */
    pop {r4-r11, lr}
    vpop {d8-d15}

    bx lr

.global __start_uctx
.type __start_uctx, %function
__start_uctx:
    /*
        After pop + vpop, sp points just past the
        100B callee-saved block.

        Stack frame built by makectx.c:
          0(sp)  = func ptr
          4(sp)  = arg
          8(sp)  = __uctx_exit (unused)
          12(sp) = ctx ptr
    */
    ldr r12, [sp, #0]       /* func ptr */
    ldr r0, [sp, #4]        /* arg */
    blx r12                 /* call func(arg), lr = return */

.global __uctx_exit
.type __uctx_exit, %function
__uctx_exit:
    ldr r0, [sp, #12]       /* ctx ptr */
    bl uctx_exit
    /* not reached */
