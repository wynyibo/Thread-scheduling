#include "task.h"
#include "console.h"
#include "init.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <fcntl.h>

#define NETLINK_USER 31

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

// 结构体来存储线程的优先级
typedef struct {
    int priority;
    char *message;
} thread_args;

int generate_random_priority()
{
    int random_value;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }
    if (read(fd, &random_value, sizeof(random_value)) != sizeof(random_value))
    {
        perror("read");
        close(fd);
        return -1;
    }
    close(fd);
    
    // 将随机数映射到 0 到 31 的范围
    return abs(random_value) % 32;
}

void *test(void *args)
{
    pid_t pid = getpid();
    thread_args *targs = (thread_args *)args;
    console_put_str(targs->message);
    return NULL;
}

void *test1(void *args)
{
    thread_args *targs = (thread_args *)args;
    while (1)
    {
        sleep(1);
        console_put_str(targs->message);
         printf("Thread  completed!\n");
    }
    return NULL;
}

/* cpu密集 */
void *test2(void *args)
{
    while (1)
        ;
}

/* io密集 */
void *test3(void *args)
{
    while (1)
    {
        sleep(1);
        console_put_str("rrr\n");
    }
}

int main()
{
    init();
    pid_t pid = getpid();
    int priority = generate_random_priority();

    // 创建 Netlink 套接字
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); // 使用当前进程的 PID

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0)
    {
        perror("bind");
        close(sock_fd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;    // 目标为内核
    dest_addr.nl_groups = 0; // 单播

    // 分配内存用于 Netlink 消息头
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(pid_t) + sizeof(int)));
    if (nlh == NULL)
    {
        perror("malloc");
        close(sock_fd);
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(sizeof(pid_t) + sizeof(int)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(pid_t) + sizeof(int));
    nlh->nlmsg_pid = getpid(); // 消息来源的进程 PID
    nlh->nlmsg_flags = 0;

    // 将 PID 和优先级复制到 Netlink 消息数据部分
    int *data = (int *)NLMSG_DATA(nlh);
    data[0] = pid;       // PID
    data[1] = priority;  // 优先级

    // 设置 iovec 结构体
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    // 设置 msghdr 结构体
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // 发送 PID 和优先级到内核模块
    sendmsg(sock_fd, &msg, 0);
    printf("Sent PID %d and priority %d to kernel module\n", pid, priority);

    const static int thread_number =  1000;
    
        // 创建一个线程使用固定消息
    thread_args *args = malloc(sizeof(thread_args));
    if (args == NULL)
    {
        perror("malloc");
        return -1;
    }
    args->priority = generate_random_priority(); // 为主线程创建不同的优先级
    args->message = "argA ";
    task_start("test1", args->priority, test, args);

    // 启动线程
    for (int i = 0; i < thread_number; i++)
    {
        thread_args *args = malloc(sizeof(thread_args));
        if (args == NULL)
        {
            perror("malloc");
            return -1;
        }
        args->priority = generate_random_priority(); // 为每个线程生成不同的优先级
        args->message = "thread ";
        task_start("abc", args->priority, test1, args);
    }
    


    while (1)
    {
        sleep(1);
        console_put_str("=================TASK===============\n");
    }

    // 释放内存
    free(nlh);
    close(sock_fd);
    return 0;
}
