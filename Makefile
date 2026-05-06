CC=clang
LD=clang

# Architecture selection
# Default: auto-detect from native architecture
# Override with: make ARCH=x86_64  or  make ARCH=aarch64
ARCH ?= $(shell $(CC) -dumpmachine | cut -d- -f1)

ARCH_DIR := arch/$(ARCH)

# armhf shares the same arch/ directory with arm (ABI differs only in target triple)
ifeq ($(ARCH),armhf)
ARCH_DIR := arch/arm
endif

# Object files in arch-specific subdirectory to avoid cross-contamination
C_SRCS = $(ARCH_DIR)/uctx.c
C_SRCS += $(ARCH_DIR)/makectx.c
C_OBJS = $(C_SRCS:.c=.o)
C_OBJS += $(ARCH_DIR)/entry.o

# Common flags
CFLAGS = -O2 -I. -I$(ARCH_DIR)
ASFLAGS = -I. -I$(ARCH_DIR)

# Architecture-specific flags
ifeq ($(ARCH),x86_64)
TARGET := --target=x86_64-linux-gnu
CFLAGS += -m64
endif
ifeq ($(ARCH),aarch64)
TARGET := --target=aarch64-linux-gnu
endif
ifeq ($(ARCH),mips)
TARGET := --target=mips-linux-gnu
endif
ifeq ($(ARCH),mipsel)
TARGET := --target=mipsel-linux-gnu
endif
ifeq ($(ARCH),arm)
TARGET := --target=arm-linux-gnueabi
endif
ifeq ($(ARCH),armhf)
TARGET := --target=arm-linux-gnueabihf
endif
ifeq ($(ARCH),riscv64)
TARGET := --target=riscv64-linux-gnu
endif
ifeq ($(ARCH),i386)
TARGET := --target=i386-linux-gnu
CFLAGS += -m32
endif

# libuctx.so is built with -nostdlib to have zero external dependencies.
# For cross-compilation targets, also use -nostdinc (types from nostdc.h).
NATIVE_ARCH := $(shell $(CC) -dumpmachine | cut -d- -f1)
ifdef TARGET
ifneq ($(ARCH),$(NATIVE_ARCH))
# Cross-compilation: use nostdc.h instead of native libc headers
CFLAGS += -nostdinc -isystem $(shell $(CC) -print-resource-dir)/include
endif
endif

# libuctx.so flags: always -nostdlib for zero dependencies
SO_CFLAGS = $(CFLAGS) -fPIC
SO_LDFLAGS = -s -flto -fPIC -nostdlib -nodefaultlibs

# test program flags: use normal libc
TEST_CFLAGS = -O2 -I. -I$(ARCH_DIR)
TEST_LDFLAGS = -L. -luctx -Wl,-rpath=.

# Zero-dependency test: no libc, uses syscall.h raw syscalls
TEST0_CFLAGS = -O2 -I. -I$(ARCH_DIR) -nostdinc -isystem $(shell $(CC) -print-resource-dir)/include
TEST0_LDFLAGS = -L. -luctx -Wl,-rpath=. -nodefaultlibs

.PHONY: all
all: libuctx.so test

.PHONY: test0
test0: test0.c libuctx.so
	$(CC) $(TARGET) $(TEST0_CFLAGS) $(TEST0_LDFLAGS) -o $@ $<

libuctx.so: $(C_OBJS)
	$(LD) $(TARGET) -shared $(SO_LDFLAGS) -o $@ $^

test: test.c libuctx.so
	$(CC) $(TARGET) $(TEST_CFLAGS) $(TEST_LDFLAGS) -o $@ $<

# Force recompilation when ARCH changes by making .o depend on a phony target
# that records the current ARCH
$(ARCH_DIR)/uctx.o: uctx.c uctx.h
	$(CC) $(TARGET) $(SO_CFLAGS) -c -o $@ $<

$(ARCH_DIR)/makectx.o: $(ARCH_DIR)/makectx.c uctx.h
	$(CC) $(TARGET) $(SO_CFLAGS) -c -o $@ $<

$(ARCH_DIR)/entry.o: $(ARCH_DIR)/entry.S $(ARCH_DIR)/asm-offset.h
	$(CC) $(TARGET) -fPIC $(ASFLAGS) -c -o $@ $<

$(ARCH_DIR)/asm-offset.h: $(ARCH_DIR)/asm-offset.s
	@$(call cmd_offsets)

$(ARCH_DIR)/asm-offset.s: asm-offset.c uctx.h
	$(CC) $(TARGET) $(SO_CFLAGS) -S $< -o $@

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
	rm -f test test0
	rm -f *.out
	rm -f *.so
	rm -f asm-offset.h
	rm -f $(ARCH_DIR)/*.o
	rm -f $(ARCH_DIR)/*.s
	rm -f $(ARCH_DIR)/asm-offset.h

.PHONY: distclean
distclean:
	rm -f test test0
	rm -f *.out
	rm -f *.so
	rm -f asm-offset.h
	rm -f arch/*/*.o
	rm -f arch/*/*.s
	rm -f arch/*/asm-offset.h
