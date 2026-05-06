#include "uctx.h"
#include <stdio.h>
#include <stdlib.h>

/* ---- test 1: float register pressure ---- */
static char sf1[8192], sf2[8192];
static struct uctx fc1, fc2, fm;
static double fc_r1, fc_r2;

static void float_coro(void *arg)
{
    double x1 = 1.111111, x2 = 2.222222, x3 = 3.333333, x4 = 4.444444;
    double x5 = 5.555555, x6 = 6.666666, x7 = 7.777777, x8 = 8.888888;
    for (int i = 0; i < 1000000; i++) {
        x1 += x2 * x3; x4 += x5 / x6; x7 += x8 - x1;
        x2 += x3 * x4; x6 += x7 / x8;
        swapuctx(arg, &fm);
    }
    double r = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
    if (arg == &fc1) fc_r1 = r; else fc_r2 = r;
    swapuctx(arg, &fm);
}

static int test_float(void)
{
    fc1.stack.ss_sp = sf1; fc1.stack.ss_size = sizeof(sf1); fc1.uc_link = NULL;
    fc2.stack.ss_sp = sf2; fc2.stack.ss_size = sizeof(sf2); fc2.uc_link = NULL;
    if (makeuctx(&fc1, float_coro, &fc1)) return 1;
    if (makeuctx(&fc2, float_coro, &fc2)) return 1;
    swapuctx(&fm, &fc1);
    swapuctx(&fm, &fc2);
    return fc_r1 != fc_r2;
}


/* ---- test 2: integer register pressure ---- */
static char si1[8192], si2[8192];
static struct uctx ic1, ic2, im;
static long ic_r1, ic_r2;

static void int_coro(void *arg)
{
    long a1=1,a2=2,a3=3,a4=4,a5=5,a6=6,a7=7,a8=8,a9=9,a10=10,a11=11,a12=12;
    for (int i = 0; i < 1000000; i++) {
        a1 += a2 * a3 - a4; a5 += a6 ^ a7; a8 += a9 << 3;
        a10 -= a11 | a12; a2 ^= a3; a12 += a1;
        swapuctx(arg, &im);
    }
    long r = a1+a2+a3+a4+a5+a6+a7+a8+a9+a10+a11+a12;
    if (arg == &ic1) ic_r1 = r; else ic_r2 = r;
    swapuctx(arg, &im);
}

static int test_integer(void)
{
    ic1.stack.ss_sp = si1; ic1.stack.ss_size = sizeof(si1); ic1.uc_link = NULL;
    ic2.stack.ss_sp = si2; ic2.stack.ss_size = sizeof(si2); ic2.uc_link = NULL;
    if (makeuctx(&ic1, int_coro, &ic1)) return 1;
    if (makeuctx(&ic2, int_coro, &ic2)) return 1;
    swapuctx(&im, &ic1);
    swapuctx(&im, &ic2);
    return ic_r1 != ic_r2;
}


/* ---- test 3: uc_link chain ---- */
static char sa[8192], sb[8192];
static struct uctx ca, cb, cm;
static int link_log;

static void c_b(void *arg) { link_log = 2; }
static void c_a(void *arg) { link_log = 1; }

static int test_uc_link(void)
{
    ca.stack.ss_sp = sa; ca.stack.ss_size = sizeof(sa); ca.uc_link = &cb;
    cb.stack.ss_sp = sb; cb.stack.ss_size = sizeof(sb); cb.uc_link = &cm;
    if (makeuctx(&ca, c_a, NULL)) return 1;
    if (makeuctx(&cb, c_b, NULL)) return 1;
    link_log = 0;
    swapuctx(&cm, &ca);
    return link_log != 2;
}


/* ---- test 4: error checks ---- */
static int test_errors(void)
{
    struct uctx bad;
    if (makeuctx(NULL, float_coro, NULL) != -1) return 1;
    if (makeuctx(&bad, NULL, NULL)      != -1) return 1;
    return 0;
}

/* ---- runner ---- */
int main(void)
{
    int fail = 0;

    if (test_float())    { printf("FAIL: float\n");    fail |= 1; }
    else                   printf("PASS: float\n");

    if (test_integer())  { printf("FAIL: integer\n");  fail |= 2; }
    else                   printf("PASS: integer\n");

    if (test_uc_link())  { printf("FAIL: uc_link\n");  fail |= 4; }
    else                   printf("PASS: uc_link\n");

    if (test_errors())   { printf("FAIL: errors\n");   fail |= 8; }
    else                   printf("PASS: errors\n");

    return fail;
}
