#include "uctx.h"
#include "nostdc.h"

extern void __swapuctx(unsigned long *, unsigned long *) __attribute__((visibility("hidden")));

__attribute__((visibility("hidden"))) void uctx_exit(struct uctx *uctx)
{
    uctx->__alive = 0;
    while (uctx->uc_link)
    {
        swapuctx(uctx, uctx->uc_link);
    }
    pthread_exit(NULL);
}

void swapuctx(struct uctx *octx, struct uctx *nctx)
{
    __swapuctx(&octx->sp, &nctx->sp);
}
