#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
.ent __swapuctx
__swapuctx:
    .set noreorder
    /*
        callee-saved registers (s0-s7, fp, ra)
        MIPS O32 ABI: s0-s7 = r16-r23, fp = r30, ra = r31
    */
    addiu $sp, $sp, -40
    sw $ra, 36($sp)
    sw $fp, 32($sp)
    sw $s7, 28($sp)
    sw $s6, 24($sp)
    sw $s5, 20($sp)
    sw $s4, 16($sp)
    sw $s3, 12($sp)
    sw $s2, 8($sp)
    sw $s1, 4($sp)
    sw $s0, 0($sp)

    /* switch stack */
    sw $sp, 0($a0)          /* *octx = sp */
    lw $sp, 0($a1)          /* sp = *nctx */

    /* restore callee-saved registers */
    lw $s0, 0($sp)
    lw $s1, 4($sp)
    lw $s2, 8($sp)
    lw $s3, 12($sp)
    lw $s4, 16($sp)
    lw $s5, 20($sp)
    lw $s6, 24($sp)
    lw $s7, 28($sp)
    lw $fp, 32($sp)
    lw $ra, 36($sp)
    addiu $sp, $sp, 40

    jr $ra
    nop                     /* delay slot */
    .set reorder
.end __swapuctx

.global __start_uctx
.type __start_uctx, @function
.ent __start_uctx
__start_uctx:
    .set noreorder
    /*
        Stack layout (from low to high address):
        0($sp)   = $s0
        4($sp)   = $s1
        ...
        36($sp)  = $ra
        40($sp)  = __start_uctx addr (return address from __swapuctx)
        44($sp)  = func ptr
        48($sp)  = arg
        52($sp)  = __uctx_exit addr
        56($sp)  = ctx ptr
    */
    lw $t0, 44($sp)         /* func ptr */
    lw $a0, 48($sp)         /* arg */
    jalr $t0                /* call func(arg) */
    nop                     /* delay slot */
    /* func returned, fall through to __uctx_exit */
    .set reorder
.end __start_uctx

.global __uctx_exit
.type __uctx_exit, @function
.ent __uctx_exit
__uctx_exit:
    .set noreorder
    lw $a0, 56($sp)         /* ctx ptr */
    j uctx_exit
    nop                     /* delay slot */
    .set reorder
.end __uctx_exit
