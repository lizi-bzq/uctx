#ifndef _STDDEF_H
#define _STDDEF_H

#include <sys/types.h>

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
