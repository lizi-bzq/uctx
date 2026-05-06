#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
__swapuctx:
    /*
        callee-saved registers:
        integer: ra, s0-s11 = 13 × 8B = 104B
        fp:      fs0-fs11  = 12 × 8B = 96B
        padding: 8B (align FP area to 16B)
        total: 208B → rounded to 240B (push + pad)
    */
    addi sp, sp, -240

    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    sd s6, 56(sp)
    sd s7, 64(sp)
    sd s8, 72(sp)
    sd s9, 80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    /* 8 bytes padding at sp+104 (unused, for 16B alignment) */

    fsd fs0, 112(sp)
    fsd fs1, 120(sp)
    fsd fs2, 128(sp)
    fsd fs3, 136(sp)
    fsd fs4, 144(sp)
    fsd fs5, 152(sp)
    fsd fs6, 160(sp)
    fsd fs7, 168(sp)
    fsd fs8, 176(sp)
    fsd fs9, 184(sp)
    fsd fs10, 192(sp)
    fsd fs11, 200(sp)

    /* switch stack */
    sd sp, 0(a0)
    ld sp, 0(a1)

    /* restore callee-saved registers */
    fld fs0, 112(sp)
    fld fs1, 120(sp)
    fld fs2, 128(sp)
    fld fs3, 136(sp)
    fld fs4, 144(sp)
    fld fs5, 152(sp)
    fld fs6, 160(sp)
    fld fs7, 168(sp)
    fld fs8, 176(sp)
    fld fs9, 184(sp)
    fld fs10, 192(sp)
    fld fs11, 200(sp)

    ld s11, 96(sp)
    ld s10, 88(sp)
    ld s9, 80(sp)
    ld s8, 72(sp)
    ld s7, 64(sp)
    ld s6, 56(sp)
    ld s5, 48(sp)
    ld s4, 40(sp)
    ld s3, 32(sp)
    ld s2, 24(sp)
    ld s1, 16(sp)
    ld s0, 8(sp)
    ld ra, 0(sp)

    addi sp, sp, 240

    ret

.global __start_uctx
.type __start_uctx, @function
__start_uctx:
    /*
        After __swapuctx restore + addi sp,240, sp points past
        the callee-saved block.

        Stack frame from makectx.c:
          0(sp)  = func ptr
          8(sp)  = arg
          16(sp) = __uctx_exit (unused; func returns via ra)
          24(sp) = ctx ptr
    */
    ld t0, 0(sp)
    ld a0, 8(sp)
    jalr t0

.global __uctx_exit
.type __uctx_exit, @function
__uctx_exit:
    ld a0, 24(sp)
    tail uctx_exit
