# uctx - User Context

一个轻量级的 x86-64 Linux 用户态协程（coroutine）库。

## 概述

uctx 提供了类似 POSIX `ucontext` 系列函数（`makecontext`/`swapcontext`）的协程功能，但实现更加精简高效。核心上下文切换由手写的 x86-64 汇编完成，仅保存/恢复 callee-saved 寄存器，避免了浮点/向量寄存器和信号掩码等不必要的开销。

## 特性

- **极简设计**：核心代码仅 ~60 行 C + 48 行汇编
- **高性能**：上下文切换仅保存/恢复 6 个 callee-saved 寄存器
- **无系统调用**：上下文切换完全在用户态完成
- **自动偏移量生成**：编译时自动计算结构体偏移
- **共享库**：编译为 `libuctx.so`，支持动态链接

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
# 编译 libuctx.so 和测试程序
make

# 运行测试
./test

# 清理构建产物
make clean
```

### 依赖

- Clang 编译器
- x86-64 Linux 环境

## 工作原理

### 栈布局

`makeuctx()` 在用户提供的栈上手动构建调用帧：

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

### 执行流程

1. `makeuctx()` 在用户栈上构建初始帧
2. 首次 `swapuctx()` 切换到协程 → 弹出寄存器 → `ret` 到 `__start_uctx` → 跳转到 `func(arg)`
3. 协程内调用 `swapuctx()` 切回主上下文
4. 协程函数返回 → `__uctx_exit` → `uctx_exit()` 清理

## 与 POSIX ucontext 对比

| 特性 | uctx | POSIX ucontext |
|------|------|----------------|
| 系统调用 | 无 | 使用 sigaltstack 等 |
| 浮点寄存器 | 不保存 | 完整保存 |
| 信号掩码 | 不处理 | 完整保存 |
| 代码量 | ~60行 C + 48行汇编 | 数百行 |
| 性能 | 更高 | 较低 |

## 许可证

GNU General Public License v2.0
