#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
.ent __swapuctx
__swapuctx:
    .set noreorder
    /*
        callee-saved registers:
        integer: s0-s7 (r16-r23), fp (r30), ra (r31) = 10 × 4B = 40B
        fp:      $f20, $f22, $f24, $f26, $f28, $f30 = 6 × 8B = 48B
        total: 88B
    */
    addiu $sp, $sp, -88

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

    sdc1 $f20, 40($sp)
    sdc1 $f22, 48($sp)
    sdc1 $f24, 56($sp)
    sdc1 $f26, 64($sp)
    sdc1 $f28, 72($sp)
    sdc1 $f30, 80($sp)

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

    ldc1 $f20, 40($sp)
    ldc1 $f22, 48($sp)
    ldc1 $f24, 56($sp)
    ldc1 $f26, 64($sp)
    ldc1 $f28, 72($sp)
    ldc1 $f30, 80($sp)

    addiu $sp, $sp, 88

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
        After __swapuctx restores callee-saved regs and
        addiu $sp, $sp, 88, sp points past the 88B block.

        Stack frame built by makectx.c:
          0($sp)  = func ptr
          4($sp)  = arg
          8($sp)  = __uctx_exit addr (unused; func returns via jalr's ra)
          12($sp) = ctx ptr
    */
    lw $t9, 0($sp)          /* func ptr -> $t9 for PIC convention */
    lw $a0, 4($sp)          /* arg */
    jalr $t9                /* call func(arg) */
    nop                     /* delay slot */
    /* func returned, fall through to __uctx_exit */
    .set reorder
.end __start_uctx

.global __uctx_exit
.type __uctx_exit, @function
.ent __uctx_exit
__uctx_exit:
    .set noreorder
    lw $a0, 12($sp)         /* ctx ptr */
    la $t9, uctx_exit
    jr $t9
    nop                     /* delay slot */
    .set reorder
.end __uctx_exit
