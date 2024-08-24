#include "task.h"
#include "analog_interrupt.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

void test(void *args)
{
    printf("task1 start:----------\n");
   
    task_exit(current_task);
}

void test1(void *args)
{
    printf("task2 start:----------\n");
   
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
