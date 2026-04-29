#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
__swapuctx:
    /*
        callee-saved registers
    */
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15

    /* switch stack */
    mov %rsp, (%rdi)
    mov (%rsi), %rsp

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    ret

.global __start_uctx
.type __start_uctx, @function
__start_uctx:
    /* co-func */
    pop %rax
    /* co arg */
    pop %rdi
    jmp *%rax

.global __uctx_exit
.type __uctx_exit, @function
__uctx_exit:
    /* the uctx */
    pop %rdi
    call uctx_exit
