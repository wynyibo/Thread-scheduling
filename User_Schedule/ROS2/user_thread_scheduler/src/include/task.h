#ifndef __TASK_H
#define __TASK_H

#include "stdint.h"
#include "list.h"
#include <setjmp.h>
#include <stdbool.h>
#include <signal.h>

typedef int16_t tid_t;
typedef void task_func(void*);

extern struct list task_ready_list;
extern struct list task_all_list;
extern struct list task_pool_list;
extern struct task_struct* current_task;

enum task_status
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_DIED,
    TASK_WAITING,
    TASK_HANGING
};

/**
 * task_stack
 * 任务自己的栈，用于存储线程中待执行的函数
 * **/
struct task_stack
{
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
};

struct task_struct
{
    uint64_t* task_stack;
    uint8_t* stack_min_addr;
    sigjmp_buf env;
    struct sigcontext context;   //save task's context
    tid_t tid;   //任务id
    enum task_status status;   //任务状态
    char name[32];   //任务名
    uint8_t priority;   //任务优先级，通过优先级设置时间片
    uint8_t ticks;   //每次处理器上执行的时间嘀嗒数，任务的时间片
    uint32_t elapsed_ticks;   //任务从开始到结束的总滴答数

    struct list_elem general_tag;
    struct list_elem all_list_tag;
    struct list_elem pool_tag; // task 池化处理

    //第一次调度的时候使用
    task_func* function;
    void* func_args;   // function(func_args);
    uint32_t stack_magic;
    unsigned int sleep_millisecond; // 用于存储超时时间，粒度为毫秒
    bool first;

    // 用于记录此协程是否用户希望被hook
    bool is_hook;

    // 用于记录此协程是否是一个时间片未跑完，但是因为调用了阻塞式系统调用而导致应该被调度
    bool is_collaborative_schedule;

    // TODO 当然以上三个成员可以分别用一个位表示，以节省字节数
};

/** 
 * task_init - 初始化任务 
 * **/
void task_init(void);

/**
 * init_task - 初始化任务基本信息
 * **/
void init_task(struct task_struct* ptask, const char* name, int prio);

/**
 * print_task_info - 打印task信息
 * **/
void print_task_info(struct task_struct* ptask);

/**
 * task_start - 创建一个优先级为prio，名字为name的任务
 * @name: 任务名
 * @prio: 任务优先级
 * @func: 任务处理函数
 * @func_arg: 任务参数
 * **/
struct task_struct* task_start(const char* name, int prio, task_func function, void* func_arg);

/**
 * tid2task - 根据tid获得task_struct
 * @tid: 任务的tid
 * **/
struct task_struct* tid2task(tid_t tid);

/**
 * schedule - 抢占式任务调度
 * **/
void schedule(unsigned long* a);

/**
 * collaborative_schedule - 协作式任务调度
 * **/
void collaborative_schedule(unsigned long* a);
void task_wait_all(void);
/**
 * task_exit - 任务结束
 * @task: 结束的任务的task_struct
 * **/
void task_exit(struct task_struct* task);

/**
 * task_block - 当前任务阻塞自己，标志其状态为status
 * @status: 转变为该状态
 * **/
void task_block(enum task_status status);

/**
 * task_unblock - 将任务ptask解除阻塞
 * @ptask: 要解除阻塞的任务结构体指针
 * **/
void task_unblock(struct task_struct* ptask);

/**
 * release_tid - 释放tid
 * @tid: 要释放的tid
 * **/
void release_tid(tid_t tid);

/**
 * 协程调用此函数，用于把hook的库函数导入进程符号表
 * **/
void co_enable_hook_sys();

/**
 * 检测当前正在运行的协程用户是否希望被hook
 * **/
bool current_is_hook();

/**
 * 定时清除死亡函数
 * **/
void clean_dead_task(void*);
#endif
