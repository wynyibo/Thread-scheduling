#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "signalkill.h"

char LICENSE[] SEC("license") = "Dual BSD/GPL";

// Ringbuffer Map to pass messages from kernel to user
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} rb SEC(".maps");

// PID Map to store the target PID from user-space
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1);
    __type(key, int);
    __type(value, int);
} pid_map SEC(".maps");

const volatile int target_ppid = 0;

SEC("tracepoint/sched/sched_switch")
int bpf_dos(struct trace_event_raw_sched_switch *ctx)
{
    long ret = 0;
    size_t pid_tgid = bpf_get_current_pid_tgid();
    int pid = pid_tgid >> 32;
    int key =0;
    int *target_pid = bpf_map_lookup_elem(&pid_map, &key);
    if(!target_pid || *target_pid !=pid)
    {
        return 0;
    }
    
    // if (target_ppid != 0) {
    //     struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    //     int ppid = BPF_CORE_READ(task, real_parent, tgid);
    //     if (ppid != target_ppid) {
    //         return 0;
    //     }
    // }

    // Send signal. 10 == SIGUSR1
    ret = bpf_send_signal(10);

    struct event *e;
    e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (e) {
        e->success = (ret == 0);
        e->pid = pid;
        bpf_get_current_comm(&e->comm, sizeof(e->comm));
        bpf_printk("%d %d %s",e->success,e->pid,e->comm);
        bpf_ringbuf_submit(e, 0);
    }

    return 0;
}