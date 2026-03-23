#ifndef _UCTX_H_
#define _UCTX_H_

struct uctx
{
    unsigned long sp;
    struct
    {
        void *ss_sp;
        unsigned long ss_size;
    } stack;
    int __alive;
    struct uctx *uc_link;
};

#ifdef __cplusplus
extern "C"
{
#endif

    extern void swapuctx(struct uctx *octx, struct uctx *nctx);
    extern int makeuctx(struct uctx *ctx, void (*func)(void *arg), void *arg);

#ifdef __cplusplus
}
#endif

#endif
