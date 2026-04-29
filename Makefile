CC=clang
LD=clang

# Architecture selection
# Default: auto-detect from native architecture
# Override with: make ARCH=x86_64  or  make ARCH=aarch64
ARCH ?= $(shell $(CC) -dumpmachine | cut -d- -f1)

ARCH_DIR := arch/$(ARCH)

C_SRCS = uctx.c
C_SRCS += $(ARCH_DIR)/makectx.c
C_OBJS = $(C_SRCS:.c=.o)
C_OBJS += $(ARCH_DIR)/entry.o

# Common flags
CFLAGS = -O2 -mgeneral-regs-only -I. -I$(ARCH_DIR)
ASFLAGS = -I. -I$(ARCH_DIR)
LDFLAGS = -s -flto -fPIC

# Architecture-specific flags
ifeq ($(ARCH),x86_64)
TARGET := --target=x86_64-linux-gnu
CFLAGS += -m64
endif
ifeq ($(ARCH),aarch64)
TARGET := --target=aarch64-linux-gnu
endif

.PHONY: all
all: libuctx.so test

libuctx.so: $(C_OBJS)
	$(LD) $(TARGET) -shared $(LDFLAGS) -o $@ $^

test: test.c libuctx.so
	$(CC) $(TARGET) -L. -luctx -Wl,-rpath=. -o $@ $<

%.o: %.c
	$(CC) $(TARGET) -fPIC $(CFLAGS) -c -o $@ $<

$(ARCH_DIR)/entry.o: $(ARCH_DIR)/entry.S $(ARCH_DIR)/asm-offset.h
	$(CC) $(TARGET) -fPIC $(ASFLAGS) -c -o $@ $<

$(ARCH_DIR)/asm-offset.h: $(ARCH_DIR)/asm-offset.s
	@$(call cmd_offsets)

$(ARCH_DIR)/asm-offset.s: $(ARCH_DIR)/asm-offset.c uctx.h
	$(CC) $(TARGET) $(CFLAGS) -S $< -o $@

define cmd_offsets
	(set -e; \
    echo "#ifndef __ASM_OFFSETS_H__"; \
    echo "#define __ASM_OFFSETS_H__"; \
    echo ""; \
    sed -n 's/.*"->\([^ ]*\) \$\?\([^ ]*\) \(.*\)".*/\#define \1 \2 \/* \3 *\//p' $<; \
    echo ""; \
    echo "#endif" ) > $@
endef

.PHONY: clean
clean:
	rm -rf *.o
	rm -f test
	rm -rf *.out
	rm -rf *.so
	rm -rf *.s
	rm -f asm-offset.h
	rm -f $(ARCH_DIR)/*.o
	rm -f $(ARCH_DIR)/*.s
	rm -f $(ARCH_DIR)/asm-offset.h

.PHONY: distclean
distclean: clean
	rm -rf arch/*/asm-offset.h arch/*/*.o arch/*/*.s
