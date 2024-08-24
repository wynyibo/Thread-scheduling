## User_Schedule项目说明文档
[TOC]
#### 项目背景

原生的Linux是支持多任务，即用户态可以创建多个Linux线程运行，但是因为Linux线程由内核统一管理，所以任务的调度粒度都是内核决定的。在一些数据处理相关的场景下，常将一个大的计算模型分解成多个小的计算任务。在原生Linux中使用Linux线程来执行多个小的计算任务，由于原生Linux线程的调度是由Linux内核决定的，所以多个计算任务的切换会花费大量的切换时间。因此，通过本项目提出的高效用户态线程调度，让多个小任务尽可能的在一个进程（单线程进程）中执行，可以减少大量不必要的切换时间，并且能够更好的利用程序的局部性原理。

基于这样的现状和需求，得出需要在用户态对任务的粒度更加细化，来更好的让任务进行处理，避免内核态下任务的频繁切换。所以提出一种Linux用户态下多任务的想法，对比目前的协程的优势和不足，计划设计出一个用户态抢占式的任务调度框架。希望达到如下目标：

- 支持用户态抢占式任务调度：多个任务之间的执行相互独立；
- 支持百万级的任务数量：能够支持大规模任务的执行；
- 支持多个调度策略：适应更多的应用场景。

#### 安装教程

```c
git clone https://gitlab.eduxiji.net/T202411664992702/project2210132-234653.git
cd src/
make all
```
>make all后生成libtask.a库文件，编译程序时在后加上该和-I ./include/库即可。

#### 使用说明

* 初始化任务框架

  `init();`

* 创建任务

  `struct task_struct* task_start(char* name, int prio, task_func function, void* func_arg);`

* 输出

   详见 `console.h`

#### 创建任务示例

 - 创建 25个任务，并让其每个任务执行100次。由于在任务创建函数中将终端屏蔽，即忽略时钟信号，以保证任务就绪链表同步，同时信号处理函数的周期为 10ms，所以当创建的任务越多，所消耗的时间就越长，稍微等待即可。配置更低的任务数可以看到更明显的效果。

```c
#include "task.h"
#include "console.h"
#include "init.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

void test(void* args)
{
    char* str = args;
    console_put_str(str);
}

void test1(void* args)
{
    char* str = args;
    while(1) {
        sleep(1);
        console_put_str(str);
    }
}

/* cpu密集 */
void test2(void* args)
{
  while(1);
}

/* io密集 */
void test3(void* args)
{
  while(1){
        sleep(1);
        console_put_str("rrr\n");
  }
}

const static int thread_number = 1000000; 

int main()
{
    struct timeval start_time,end_time;
    gettimeofday(&start_time,NULL);//获取开始时间
    init();
    task_start("tast1", 31, test, "argA ");
    for(int i = 0; i < thread_number; i++) {
        task_start("abc", 31, test1, "a ");
    }
    gettimeofday(&end_time,NULL);//获取结束时间
    int loop_count = 100;
    while(loop_count--) {
        sleep(1);
        console_put_str("maiN \n");
    }
    gettimeofday(&end_time,NULL);//获取结束时间
    //计算时间
    long seconds = end_time.tv_sec - start_time.tv_sec;//秒差
    long microseconds = end_time.tv_usec - start_time.tv_usec;//微妙差

    //去掉负值
    if(microseconds < 0)
    {
     /*
     由于结束的微妙可能小于开始的微妙值，差值就会为负值
     需要借秒，减去1秒，加到微妙上面。
     */
        seconds -= 1;
        microseconds += 1000000;
    }

    //秒
    double mintus = seconds + microseconds / 1000000.0;

    //打印其时间差
    printf("Consumption of time time: %.3f seconds.\n", mintus);

    return 0;
}
```

* 编译：
```bash
cd src
make clean && make all
gcc main.c -o main -Wl,--no-as-needed -ldl -I include/ libtask.a
```

* 运行
```c
./main
```

* 一键编译运行
```c
./run.sh
```

#### Hook 模块
Hook模块应用于task中调用阻塞式系统调用的场景，此时task不应该继续执行原本分配的时间片，而应该主动让出自己的执行权，并在所等待时间就绪时重新加入全局就绪队列，在一个多任务操作系统环境中模拟创建并运行不同类型的任务，让主任务处于阻塞状态，调度其他任务的场景。

```c
#include "task.h"
#include "analog_interrupt.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

void test(void *args)
{
    printf("task1  start:----------\n");

    task_exit(current_task);

}

void test1(void *args)
{
    printf("task2  start:----------\n");

    task_exit(current_task);
    }

int main()
{

    init();
    
    print_task_info(current_task);
    task_start("test", 31, test, NULL);
    task_start("test1", 31, test1, NULL);
    struct task_struct *ptask = tid2task(2);
    print_task_info(ptask);
    ptask = tid2task(3);
    print_task_info(ptask);
    interrupt_enable();
    
    while (1)
    {
         pause();
    }
    
     return 0;

}
```

* 编译：
```c
cd src
rm libtask.a
make all
gcc schedule_main.c -o schedule_main -Wl,--no-as-needed -ldl -I include/ libtask.a
```

* 运行
```c
./schedule_main
```
