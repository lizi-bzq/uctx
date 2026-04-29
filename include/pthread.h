#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <sys/types.h>

__attribute__((__noreturn__))
void pthread_exit(void *retval);

#endif
