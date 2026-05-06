#include "asm-offset.h"

.text

.extern uctx_exit

.global __swapuctx
.type __swapuctx, @function
__swapuctx:
    /*
        callee-saved: ebx, esi, edi, ebp = 16B

        i386 cdecl: args on stack at 4(%esp) and 8(%esp).
        After 4 pushes (16B), offsets shift by +16.
    */
    push %ebp
    push %edi
    push %esi
    push %ebx

    /* load arguments (now at esp+20, esp+24) */
    mov 20(%esp), %eax      /* &octx->sp */
    mov 24(%esp), %edx      /* &nctx->sp */

    /* switch stack */
    mov %esp, (%eax)
    mov (%edx), %esp

    /* restore */
    pop %ebx
    pop %esi
    pop %edi
    pop %ebp
    ret

.global __start_uctx
.type __start_uctx, @function
__start_uctx:
    /*
        After ret, esp points to:
          0(%esp)  = func ptr
          4(%esp)  = __uctx_exit (func's return addr, pre-pushed)
          8(%esp)  = arg
          12(%esp) = ctx ptr

        i386 cdecl: callee expects 1st arg at 4(%esp) after call.
        Since we jmp (not call), the pre-pushed __uctx_exit acts
        as the return address, and arg sits at 4(%esp) — correct.
    */
    pop %eax            /* func ptr */
    jmp *%eax           /* jump to func; its ret pops __uctx_exit */

.global __uctx_exit
.type __uctx_exit, @function
__uctx_exit:
    /*
        After func returns, esp points past the __uctx_exit slot:
          0(%esp)  = arg (unused now)
          4(%esp)  = ctx ptr

        Load ctx and call uctx_exit.
    */
    mov 4(%esp), %eax   /* ctx ptr */
    push %eax            /* arg for uctx_exit */
    call uctx_exit
