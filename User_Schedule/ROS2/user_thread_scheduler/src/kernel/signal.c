#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/tracepoint.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/sched/signal.h>
#include <linux/workqueue.h>

#define NETLINK_USER 31
#define TICKS 0

// 定义一个 Netlink 套接字，用于内核和用户空间之间的通信
static struct sock *nl_sk = NULL;

static int target_pid = -1;
static int ticks = TICKS;
static struct interest_tracepoint
{
    void *callback;
    struct tracepoint *ptr;
    char is_registered; // 标记 tracepoint 是否已注册
} sched_switch_tracepoint = {.callback = NULL, .ptr = NULL, .is_registered = 0};

// 宏定义，查找指定名字的 tracepoint
#define TRACEPOINT_FIND(tracepoint_name)                                             \
    static void tracepoint_name##_tracepoint_find(struct tracepoint *tp, void *priv) \
    {                                                                                \
        if (!strcmp(#tracepoint_name, tp->name))                                     \
        {                                                                            \
            ((struct interest_tracepoint *)priv)->ptr = tp;                          \
            return;                                                                  \
        }                                                                            \
    }

// 延迟工作队列和工作项
static struct delayed_work delayed_work;
static struct workqueue_struct *wq;

// 用于注销 tracepoint 的回调函数
static void clear_tracepoint(struct interest_tracepoint *interest)
{
    if (interest->is_registered)
    {
        tracepoint_probe_unregister(interest->ptr, interest->callback, NULL);
        interest->is_registered = 0; // 更新状态
    }
}

// 生成用于查找 sched_switch tracepoint 的函数
TRACEPOINT_FIND(sched_switch)

// 延迟工作队列的工作函数，阻塞指定 PID 的 SIGUSR1 信号
static void block_sigusr1_for_pid(struct work_struct *work)
{
    struct task_struct *task;
    sigset_t new_mask;

    // 读取 RCU 保护的任务结构体
    rcu_read_lock();
    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);

    if (task)
    {
        if (task->pid != target_pid)
        {
            rcu_read_unlock();
            return;
        }
        get_task_struct(task);
        rcu_read_unlock();

        printk(KERN_INFO "Blocking SIGUSR1 for process %s [%d]\n", task->comm, task->pid);
        if (ticks == 0)
        {
    //          struct kernel_siginfo info;
    //          char *message = "sigusr1 is success";
    //         //  memset(&info, 0, sizeof(struct siginfo));
    // info.si_signo = SIGUSR1;
    // info.si_code = SI_KERNEL;  // 表明信号来自内核
    // info.si_ptr = (void *)message;  // 你可以设置一些额外的信息
            // 初始化新的信号集，阻塞 SIGUSR1
            sigemptyset(&new_mask);
            sigaddset(&new_mask, SIGUSR1);

            // 修改进程的信号阻塞集
            spin_lock_irq(&task->sighand->siglock);
            // send_sig_info(SIGUSR1, &info, task);
            sigaddset(&task->blocked, SIGUSR1); // 将 SIGUSR1 添加到阻塞信号集
            recalc_sigpending();                // 更新挂起的信号
            spin_unlock_irq(&task->sighand->siglock);

            printk(KERN_INFO "SIGUSR1 blocked for process %s [%d]\n", task->comm, task->pid);
        }
        put_task_struct(task);
    }
    else
    {
        rcu_read_unlock();
        pr_err("Task with PID %d not found\n", target_pid);
    }
    printk(KERN_INFO "Current ticks value in block_sigusr1_for_pid: %d\n", ticks);
    if (ticks > 0)
    {
        // 重新排队延迟工作
        queue_delayed_work(wq, &delayed_work, msecs_to_jiffies(1000)); // 1 秒延迟
    }
}

// tracepoint 的回调函数
static void sched_switch_tracepoint_callback(void *data, bool preempt, struct task_struct *prev, struct task_struct *next)
{
    if (next->pid != target_pid)
        return;
    printk(KERN_INFO "sched_switch event\n");

    if (ticks > 0)
    {
        ticks--;
        // printk(KERN_INFO "Ticks remaining: %d\n", ticks);

        if (ticks == 0)
        {
            queue_delayed_work(wq, &delayed_work, 0);
        }
    }
}

// Netlink 套接字接收到消息后的处理函数
static void netlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int msg_size;
    int res;
    u32 sender_pid;
    int *data;

    nlh = (struct nlmsghdr *)skb->data;
    sender_pid = nlh->nlmsg_pid; // 获取发送者的 PID

    data = (int *)nlmsg_data(nlh);
    target_pid = data[0]; // 获取 PID
    ticks = data[1];      // 获取优先级作为 ticks 值
    printk(KERN_INFO "Received PID: %d, Priority: %d\n", target_pid, ticks);

    msg_size = sizeof(int);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out)
    {
        pr_err("Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    *(int *)nlmsg_data(nlh) = target_pid;

    res = nlmsg_unicast(nl_sk, skb_out, sender_pid); // 使用获取的 PID
    if (res < 0)
        pr_err("Error while sending back to user\n");
}

// 内核模块初始化函数
static int __init netlink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = netlink_recv_msg,
    };

    // 创建 Netlink 套接字
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk)
    {
        pr_err("Error creating netlink socket\n");
        return -10;
    }

    // 创建工作队列
    wq = create_singlethread_workqueue("sigusr1_block_wq");
    if (!wq)
    {
        pr_err("Error creating workqueue\n");
        netlink_kernel_release(nl_sk); // 释放已分配的 Netlink 套接字
        return -ENOMEM;
    }

    // 初始化延迟工作项
    INIT_DELAYED_WORK(&delayed_work, block_sigusr1_for_pid);
    // 排队工作项，延迟 1 秒执行
    queue_delayed_work(wq, &delayed_work, msecs_to_jiffies(1000)); // 1 秒延迟

    // 查找 sched_switch tracepoint
    sched_switch_tracepoint.callback = sched_switch_tracepoint_callback;
    // 查找并注册 tracepoint
    for_each_kernel_tracepoint(sched_switch_tracepoint_find, &sched_switch_tracepoint);

    if (!sched_switch_tracepoint.ptr)
    {
        pr_info("sched_switch's struct tracepoint not found\n");
        destroy_workqueue(wq);         // 销毁工作队列
        netlink_kernel_release(nl_sk); // 释放 Netlink 套接字
        return 0;
    }

    // 注册 tracepoint
    if (tracepoint_probe_register(sched_switch_tracepoint.ptr, sched_switch_tracepoint.callback, NULL))
    {
        pr_err("Failed to register tracepoint\n");
        clear_tracepoint(&sched_switch_tracepoint);
        destroy_workqueue(wq);         // 销毁工作队列
        netlink_kernel_release(nl_sk); // 释放 Netlink 套接字
        return -EINVAL;
    }

    sched_switch_tracepoint.is_registered = 1;
    return 0;
}

// 内核模块卸载函数
static void __exit netlink_exit(void)
{
    // 注销 tracepoint
    clear_tracepoint(&sched_switch_tracepoint);
    // 取消延迟工作项并同步
    cancel_delayed_work_sync(&delayed_work);
    // 销毁工作队列
    destroy_workqueue(wq);
    // 释放 Netlink 套接字
    netlink_kernel_release(nl_sk);
}

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Netlink kernel module to receive and send back PID with filtering");
MODULE_AUTHOR("linux");