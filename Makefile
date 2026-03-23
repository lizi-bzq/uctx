CC=clang
LD=clang

C_SRCS=uctx.c
C_OBJS=$(C_SRCS:.c=.o) entry.o

CFLAGS=-O2 -mgeneral-regs-only
LDFLAGS=-s -flto -fPIC


sed-y='s/.*"->\([^ ]*\) \$\([0-9]*\) \(.*\)".*/\#define \1 \2 \/* \3 *\//p'

define cmd_offsets
	(set -e; \
    echo "#ifndef __ASM_OFFSETS_H__"; \
    echo "#define __ASM_OFFSETS_H__"; \
    echo ""; \
    sed -n 's/.*"->\([^ ]*\) \$$\([^ ]*\) \(.*\)".*/\#define \1 \2 \/* \3 *\//p' $<; \
    echo ""; \
    echo "#endif" ) > $@
endef 


.PHONY:all
all: libuctx.so test

libuctx.so:$(C_OBJS)
	$(LD) -shared $(LDFLAGS) -o $@ $^

test:test.c libuctx.so
	$(CC) -L. -luctx -Wl,-rpath=. -o $@ $^
	
%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c -o $@ $^

entry.o: entry.S asm-offset.h
	$(CC) -fPIC -c -o $@ $<

asm-offset.h: asm-offset.s
	@$(call cmd_offsets)
asm-offset.s: asm-offset.c
	$(CC) -S $<

.PHONY:clean
clean:
	rm -rf *.o
	rm -f test
	rm -rf *.out
	rm -rf *.so
	rm -rf *.s
	rm -f asm-offset.h
