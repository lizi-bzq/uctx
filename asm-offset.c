#include "uctx.h"
#include "nostdc.h"

#define DEFINE(sym, val) asm volatile("\n.ascii \"->" #sym " %0 " #val "\"" : : "i"(val))
#define OFFSET(sym, str, mem) DEFINE(sym, offsetof(struct str, mem))

__attribute__((__used__)) static void common(void)
{
    OFFSET(uctx_sp, uctx, sp);
}
