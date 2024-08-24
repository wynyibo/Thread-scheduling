# 概要

我们选取了最简单的for循环作为研究对象，来观察使用和未使用openMP时程序执行流程的差别，这里使用`strace`命令对程序的系统调用进行跟踪。

# 代码

- [for.c](for.c)：使用了openMP的for循环代码
- [norm_for.c](norm_for.c)：未使用openMP的for循环代码

# 跟踪

这里使用`strace`命令对上述代码编译出的程序进行跟踪，产生了如下两个文件：

- [for.tarce](for.trace)
- [norm_for.tarce](norm_for.trace)

# 差异

这里使用`diff`命令对比两者执行流程的差异：

```shell
$ diff norm_for.trace for.trace 
1,4c1,4
< execve("./norm_for", ["./norm_for"], 0x7ffccad8b050 /* 48 vars */) = 0
< brk(NULL)                               = 0x5d8c00751000
< arch_prctl(0x3001 /* ARCH_??? */, 0x7ffd11fffc50) = -1 EINVAL (Invalid argument)
< mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x72520b34d000
---
> execve("./for", ["./for"], 0x7fff255d99b0 /* 48 vars */) = 0
> brk(NULL)                               = 0x651bfb9de000
> arch_prctl(0x3001 /* ARCH_??? */, 0x7ffe6e4ba930) = -1 EINVAL (Invalid argument)
> mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x785097749000
8c8,16
< mmap(NULL, 47203, PROT_READ, MAP_PRIVATE, 3, 0) = 0x72520b341000
---
> mmap(NULL, 47203, PROT_READ, MAP_PRIVATE, 3, 0) = 0x78509773d000
> close(3)                                = 0
> openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libgomp.so.1", O_RDONLY|O_CLOEXEC) = 3
> read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
> newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=339848, ...}, AT_EMPTY_PATH) = 0
> mmap(NULL, 338320, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7850976ea000
> mmap(0x7850976f5000, 229376, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0xb000) = 0x7850976f5000
> mmap(0x78509772d000, 57344, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x43000) = 0x78509772d000
> mmap(0x78509773b000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x51000) = 0x78509773b000
15,19c23,27
< mmap(NULL, 2150256, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x72520b000000
< mmap(0x72520b026000, 1568768, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x26000) = 0x72520b026000
< mmap(0x72520b1a5000, 348160, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1a5000) = 0x72520b1a5000
< mmap(0x72520b1fa000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1f9000) = 0x72520b1fa000
< mmap(0x72520b200000, 53104, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x72520b200000
---
> mmap(NULL, 2150256, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x785097400000
> mmap(0x785097426000, 1568768, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x26000) = 0x785097426000
> mmap(0x7850975a5000, 348160, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1a5000) = 0x7850975a5000
> mmap(0x7850975fa000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1f9000) = 0x7850975fa000
> mmap(0x785097600000, 53104, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x785097600000
21,28c29,37
< mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x72520b33e000
< arch_prctl(ARCH_SET_FS, 0x72520b33e740) = 0
< set_tid_address(0x72520b33ea10)         = 35979
< set_robust_list(0x72520b33ea20, 24)     = 0
< rseq(0x72520b33f060, 0x20, 0, 0x53053053) = 0
< mprotect(0x72520b1fa000, 16384, PROT_READ) = 0
< mprotect(0x5d8c003d6000, 4096, PROT_READ) = 0
< mprotect(0x72520b384000, 8192, PROT_READ) = 0
---
> mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7850976e7000
> arch_prctl(ARCH_SET_FS, 0x7850976e77c0) = 0
> set_tid_address(0x7850976e7a90)         = 36021
> set_robust_list(0x7850976e7aa0, 24)     = 0
> rseq(0x7850976e80e0, 0x20, 0, 0x53053053) = 0
> mprotect(0x7850975fa000, 16384, PROT_READ) = 0
> mprotect(0x78509773b000, 4096, PROT_READ) = 0
> mprotect(0x651bfb05f000, 4096, PROT_READ) = 0
> mprotect(0x785097780000, 8192, PROT_READ) = 0
30c39,66
< munmap(0x72520b341000, 47203)           = 0
---
> munmap(0x78509773d000, 47203)           = 0
> openat(AT_FDCWD, "/sys/devices/system/cpu/possible", O_RDONLY|O_CLOEXEC) = 3
> read(3, "0-127\n", 1024)                = 6
> close(3)                                = 0
> getrandom("\x17\x24\xa7\xa4\x2b\x86\x50\x23", 8, GRND_NONBLOCK) = 8
> brk(NULL)                               = 0x651bfb9de000
> brk(0x651bfb9ff000)                     = 0x651bfb9ff000
> sched_getaffinity(36021, 16, [0 1 2 3]) = 16
> rt_sigaction(SIGRT_1, {sa_handler=0x7850974948a0, sa_mask=[], sa_flags=SA_RESTORER|SA_ONSTACK|SA_RESTART|SA_SIGINFO, sa_restorer=0x785097442990}, NULL, 8) = 0
> rt_sigprocmask(SIG_UNBLOCK, [RTMIN RT_1], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x785096bff000
> mprotect(0x785096c00000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x7850973ff990, parent_tid=0x7850973ff990, exit_signal=0, stack=0x785096bff000, stack_size=0x7fff00, tls=0x7850973ff6c0} => {parent_tid=[36022]}, 88) = 36022
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x7850963fe000
> mprotect(0x7850963ff000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x785096bfe990, parent_tid=0x785096bfe990, exit_signal=0, stack=0x7850963fe000, stack_size=0x7fff00, tls=0x785096bfe6c0} => {parent_tid=[36023]}, 88) = 36023
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x785095bfd000
> mprotect(0x785095bfe000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x7850963fd990, parent_tid=0x7850963fd990, exit_signal=0, stack=0x785095bfd000, stack_size=0x7fff00, tls=0x7850963fd6c0} => {parent_tid=[36024]}, 88) = 36024
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> futex(0x651bfb9de374, FUTEX_WAKE_PRIVATE, 2147483647) = 2
> futex(0x651bfb9de484, FUTEX_WAKE_PRIVATE, 2147483647) = 0
> futex(0x651bfb9de484, FUTEX_WAKE_PRIVATE, 2147483647) = 0
```

可见使用了openMP的程序使用了更多的系统调用，主要有以下几个部分：

```diff
> close(3)                                = 0
> openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libgomp.so.1", O_RDONLY|O_CLOEXEC) = 3
> read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 832) = 832
> newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=339848, ...}, AT_EMPTY_PATH) = 0
> mmap(NULL, 338320, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7850976ea000
> mmap(0x7850976f5000, 229376, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0xb000) = 0x7850976f5000
> mmap(0x78509772d000, 57344, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x43000) = 0x78509772d000
> mmap(0x78509773b000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x51000) = 0x78509773b000
```
使用了`/lib/x86_64-linux-gnu/libgomp.so.1`动态链接库。

```diff
> openat(AT_FDCWD, "/sys/devices/system/cpu/possible", O_RDONLY|O_CLOEXEC) = 3
> read(3, "0-127\n", 1024)                = 6
> close(3)                                = 0
> getrandom("\x17\x24\xa7\xa4\x2b\x86\x50\x23", 8, GRND_NONBLOCK) = 8
> brk(NULL)                               = 0x651bfb9de000
> brk(0x651bfb9ff000)                     = 0x651bfb9ff000
> sched_getaffinity(36021, 16, [0 1 2 3]) = 16
```
获取核心数，以及调整亲和性。

```diff
> rt_sigaction(SIGRT_1, {sa_handler=0x7850974948a0, sa_mask=[], sa_flags=SA_RESTORER|SA_ONSTACK|SA_RESTART|SA_SIGINFO, sa_restorer=0x785097442990}, NULL, 8) = 0
> rt_sigprocmask(SIG_UNBLOCK, [RTMIN RT_1], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x785096bff000
> mprotect(0x785096c00000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x7850973ff990, parent_tid=0x7850973ff990, exit_signal=0, stack=0x785096bff000, stack_size=0x7fff00, tls=0x7850973ff6c0} => {parent_tid=[36022]}, 88) = 36022
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x7850963fe000
> mprotect(0x7850963ff000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x785096bfe990, parent_tid=0x785096bfe990, exit_signal=0, stack=0x7850963fe000, stack_size=0x7fff00, tls=0x785096bfe6c0} => {parent_tid=[36023]}, 88) = 36023
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x785095bfd000
> mprotect(0x785095bfe000, 8388608, PROT_READ|PROT_WRITE) = 0
> rt_sigprocmask(SIG_BLOCK, ~[], [], 8)   = 0
> clone3({flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, child_tid=0x7850963fd990, parent_tid=0x7850963fd990, exit_signal=0, stack=0x785095bfd000, stack_size=0x7fff00, tls=0x7850963fd6c0} => {parent_tid=[36024]}, 88) = 36024
> rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
> futex(0x651bfb9de374, FUTEX_WAKE_PRIVATE, 2147483647) = 2
> futex(0x651bfb9de484, FUTEX_WAKE_PRIVATE, 2147483647) = 0
> futex(0x651bfb9de484, FUTEX_WAKE_PRIVATE, 2147483647) = 0
```
fork出线程以及使用锁机制。