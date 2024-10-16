#include "console.h"
#include "sync.h"
#include "task.h"
#include "ioqueue.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static struct lock concole_lock;   //控制台锁

static char io_buffer[1024];
static int fionbio_attr = 1;

/**
 * console_init - 初始化终端
 * **/
void console_init()
{
    lock_init(&concole_lock);
    //设置输入输出为非阻塞
    ioctl(STDIN_FILENO, FIONBIO, &fionbio_attr);
    ioctl(STDOUT_FILENO, FIONBIO, &fionbio_attr);
}

/**
 * console_acquire - 获取终端
 * **/
void console_acquire()
{
    lock_acquire(&concole_lock);
}

/**
 * console_release - 释放终端
 * **/
void console_release()
{
    lock_release(&concole_lock);
}

/**
 * console_put_str - 输出字符串
 * **/
void console_put_str(const char* str)
{
    console_acquire();
    //输出字符串
    write(STDOUT_FILENO, str, strlen(str));
    console_release();
}

/**
 * console_put_int - 输出int型数字十进制
 * **/
void console_put_int(int n)
{
    console_acquire();
    // char str[100];
    // itoa(n, str, 10);
    // write(STDOUT_FILENO, str, strlen(str));
    printf("%d", n);
    console_release();
}

/**
 * console_put_char - 输出字符
 * **/
void console_put_char(char ch)
{
    console_acquire();
    write(STDOUT_FILENO, &ch, 1);
    console_release();
}

void console_get_str(char* str)
{
    console_acquire();
    read(STDIN_FILENO, str, sizeof(io_buffer));
    console_release();
}
