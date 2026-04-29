# uctx - User Context

一个轻量级的用户态协程（coroutine）库，支持多架构。

## 概述

uctx 提供了类似 POSIX `ucontext` 系列函数（`makecontext`/`swapcontext`）的协程功能，但实现更加精简高效。核心上下文切换由手写汇编完成，仅保存/恢复 callee-saved 寄存器，避免了浮点/向量寄存器和信号掩码等不必要的开销。

## 特性

- **极简设计**：核心代码仅 ~60 行 C + 少量汇编
- **高性能**：上下文切换仅保存/恢复 callee-saved 寄存器
- **无系统调用**：上下文切换完全在用户态完成
- **自动偏移量生成**：编译时自动计算结构体偏移
- **共享库**：编译为 `libuctx.so`，支持动态链接
- **多架构**：支持 x86_64、AArch64、MIPS (big/little endian)
- **零外部依赖**：`libuctx.so` 不依赖任何外部库（`NEEDED` 为空）

## API

```c
struct uctx {
    unsigned long sp;          // 栈指针
    struct {
        void *ss_sp;           // 栈基址
        unsigned long ss_size; // 栈大小
    } stack;
    int __alive;               // 协程是否存活
    struct uctx *uc_link;      // 链接到下一个上下文
};

// 创建协程上下文
int makeuctx(struct uctx *ctx, void (*func)(void *), void *arg);

// 切换协程上下文
void swapuctx(struct uctx *octx, struct uctx *nctx);
```

## 构建

```bash
# 编译本机架构的 libuctx.so 和测试程序
make

# 编译指定架构
make ARCH=x86_64
make ARCH=aarch64
make ARCH=mips
make ARCH=mipsel

# 运行测试（本机或 QEMU 用户态模拟）
./test
qemu-x86_64 -L /usr/x86_64-linux-gnu ./test
qemu-aarch64 -L /usr/aarch64-linux-gnu ./test
qemu-mips -L /usr/mips-linux-gnu ./test

# 清理构建产物
make clean
```

### 依赖

- Clang 编译器
- 目标架构的 QEMU 用户态模拟器（用于交叉编译测试）

## 工作原理

### 栈布局

`makeuctx()` 在用户提供的栈上手动构建调用帧：

**x86_64:**
```
高地址
+------------------+
|      0           |  ← 16字节对齐
|    ctx 指针      |  → 传给 __uctx_exit
|  __uctx_exit 地址 |  → 函数返回地址
|     arg          |  → 传给 func 的参数
|   func 地址      |  → 传给 __start_uctx
| __start_uctx 地址 |  → 首次 ret 的目标
|     rbp = 0      |
|     rbx = 0      |
|     r12 = 0      |
|     r13 = 0      |
|     r14 = 0      |
|     r15 = 0      |  ← sp 指向这里
+------------------+
低地址
```

**AArch64:**
```
高地址
+------------------+
|    ctx 指针      |  → 传给 __uctx_exit
|     arg          |  → 传给 func 的参数
|   func 地址      |  → 传给 __start_uctx
|     x30          |  = __start_uctx（首次 eret 的目标）
|     x29 = 0      |
|     x28 = 0      |
|     ...          |
|     x19 = 0      |  ← sp 指向这里
+------------------+
低地址
```

**MIPS (O32 ABI):**
```
高地址
+------------------+
|    ctx 指针      |  → 传给 __uctx_exit
|  __uctx_exit 地址 |  → func 返回后的地址
|     arg          |  → 传给 func 的参数 ($a0)
|   func 地址      |  → 传给 __start_uctx
| __start_uctx 地址 |  → 首次 jr $ra 的目标
|     $s7 = 0      |
|     $s6 = 0      |
|     ...          |
|     $s0 = 0      |
|     $fp = 0      |
|     $ra = 0      |  ← sp 指向这里
+------------------+
低地址
```

### 执行流程

1. `makeuctx()` 在用户栈上构建初始帧
2. 首次 `swapuctx()` 切换到协程 → 弹出寄存器 → 跳转到 `__start_uctx` → 跳转到 `func(arg)`
3. 协程内调用 `swapuctx()` 切回主上下文
4. 协程函数返回 → `__uctx_exit` → `uctx_exit()` 清理

## 与 POSIX ucontext 对比

| 特性 | uctx | POSIX ucontext |
|------|------|----------------|
| 系统调用 | 无 | 使用 sigaltstack 等 |
| 浮点寄存器 | 不保存 | 完整保存 |
| 信号掩码 | 不处理 | 完整保存 |
| 代码量 | ~60行 C + 少量汇编 | 数百行 |
| 性能 | 更高 | 较低 |
| 外部依赖 | 无（libc 仅用于测试） | 需要 libc |

## 项目结构

```
├── uctx.h              # 公共头文件
├── uctx.c              # 核心 C 代码
├── test.c              # 测试程序
├── Makefile            # 构建系统
├── include/            # 最小化标准库头文件（交叉编译用）
│   ├── pthread.h
│   ├── stdio.h
│   ├── stddef.h
│   ├── time.h
│   ├── ucontext.h
│   └── sys/
│       └── types.h
└── arch/
    ├── x86_64/         # x86_64 实现
    │   ├── entry.S     # 上下文切换汇编
    │   ├── makectx.c   # 栈帧构建
    │   └── asm-offset.c
    ├── aarch64/        # AArch64 实现
    │   ├── entry.S
    │   ├── makectx.c
    │   └── asm-offset.c
    └── mips/           # MIPS 实现（大端/小端通用）
        ├── entry.S
        ├── makectx.c
        └── asm-offset.c
```

## 许可证

GNU General Public License v2.0
