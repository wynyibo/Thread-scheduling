#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <omp.h>

#define THREAD_NUMBER 25

// 获取线程的 PID
pid_t get_thread_id() {
    return syscall(SYS_gettid);
}

// 用于打印字符串的函数
void print_message(const char *message) {
    pid_t thread_id = get_thread_id();
    
    while (1) {
        sleep(1);  // 休眠 1 秒
        printf("Thread ID: %d - %s\n", thread_id, message);
    }
}

// 用于 CPU 密集型任务的函数
void cpu_intensive_task() {
    pid_t thread_id = get_thread_id();
    
    while (1) {
        // 空循环，CPU 密集型任务
        // 可以在这里添加代码打印 CPU 密集型任务的 ID
        // printf("CPU Intensive Thread ID: %d\n", thread_id);
    }
}

// 用于 IO 密集型任务的函数
void io_intensive_task() {
    pid_t thread_id = get_thread_id();
    
    while (1) {
        sleep(1);  // 休眠 1 秒
        printf("IO Intensive Thread ID: %d - IO intensive task\n", thread_id);
    }
}

int main() {
    // 初始化 OpenMP
    #pragma omp parallel
    {
        #pragma omp single
        {
            // 创建一个单独的任务，用于 IO 密集型任务
            #pragma omp task
            {
                // 打印该任务的线程 ID
                printf("Task ID: %d - IO Intensive Single thread running...\n", get_thread_id());
                io_intensive_task();
            }

            // 创建 25 个任务，用于 IO 密集型任务
            #pragma omp parallel
            {
                #pragma omp single
                {
                    for (int i = 0; i < THREAD_NUMBER; i++) {
                        #pragma omp task
                        {
                            // 打印每个任务的线程 ID
                            printf("Task ID: %d - IO Intensive Thread running...\n", get_thread_id());
                            io_intensive_task();
                        }
                    }
                }
            }
        }
    }

    return 0;
}
