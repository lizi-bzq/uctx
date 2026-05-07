#ifndef _NOSTDC_H_
#define _NOSTDC_H_

/* sys/types.h — basic types */
typedef unsigned long size_t;
typedef __SIZE_TYPE__ ssize_t;
#define NULL ((void *)0)

/* stddef.h — offsetof */
#define offsetof(type, member) __builtin_offsetof(type, member)

/* pthread.h — pthread_exit */
__attribute__((__noreturn__)) void pthread_exit(void *retval);

#endif
