#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define THREAD_NUMBER 25

// 获取线程的 PID
pid_t get_thread_id() {
    return syscall(SYS_gettid);
}

// 用于打印字符串的函数
void *print_message(void *arg) {
    char *message = (char *)arg;
    pid_t thread_id = get_thread_id();
    
    while (1) {
        sleep(1);  // 休眠 1 秒
        printf("Thread ID: %d - %s\n", thread_id, message);
    }
    return NULL;
}

// 用于 CPU 密集型任务的函数
void *cpu_intensive_task(void *arg) {
    pid_t thread_id = get_thread_id();
    
    while (1) {
        // 空循环，CPU 密集型任务
        // 可以在这里添加代码打印 CPU 密集型任务的 ID
        // printf("CPU Intensive Thread ID: %d\n", thread_id);
    }
    return NULL;
}

// 用于 IO 密集型任务的函数
void *io_intensive_task(void *arg) {
    pid_t thread_id = get_thread_id();
    
    while (1) {
        sleep(1);  // 休眠 1 秒
        printf("IO Intensive Thread ID: %d - IO intensive task\n", thread_id);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_NUMBER];
    int i;
    
    // 创建一个单独的线程
    pthread_t single_thread;
    const char *message = "Single thread running...";
    if (pthread_create(&single_thread, NULL, print_message, (void *)message)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // 创建 25 个线程
    for (i = 0; i < THREAD_NUMBER; i++) {
        if (pthread_create(&threads[i], NULL, print_message, "Thread running...")) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    // 等待所有线程完成
    for (i = 0; i < THREAD_NUMBER; i++) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "Error joining thread %d\n", i);
            return 1;
        }
    }

    // 等待单独的线程完成
    if (pthread_join(single_thread, NULL)) {
        fprintf(stderr, "Error joining single thread\n");
        return 1;
    }

    return 0;
}
