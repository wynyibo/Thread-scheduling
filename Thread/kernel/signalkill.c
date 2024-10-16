#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include "../include/task.h"
#include "../include/console.h"
#include "../include/init.h"
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <bpf/bpf.h>

#include "signalkill.h"
#include "signalkill.skel.h"

static volatile bool exiting = false;

int count = 0;
int count_i = 0;
bool verbose = false;

int sport, dport, sampling, local;

const char argp_program_doc[] = "Trace time delay in network subsystem \n";

static const struct argp_option opts[] = {
    {"verbose", 'v', NULL, 0, "Verbose debug output"},
    {},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{

    switch (key)
    {
    case 'v':
        verbose = true;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static const struct argp argp = {
    .options = opts,
    .parser = parse_arg,
    .doc = argp_program_doc,
};

static void sig_handler(int sig)
{
    //signal_header(sig);
}

void test(void *args)
{
    char *str = args;
    console_put_str(str);
}

void test1(void *args)
{
    char *str = args;
    while (1)
    {
        sleep(1);
        console_put_str(str);
    }
}

/* cpu密集 */
void test2(void *args)
{
    while (1)
        ;
}

/* io密集 */
void test3(void *args)
{
    while (1)
    {
        sleep(1);
        console_put_str("rrr\n");
    }
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    if (level == LIBBPF_DEBUG && !verbose)
        return 0;
    return vfprintf(stderr, format, args);
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
    const struct event *d = data;
    printf("%-20d %-20d %-20s\n", d->pid, d->success, d->comm);
    return 0;
}

int main(int argc, char **argv)
{

    struct ring_buffer *rb = NULL;
    struct signalkill_bpf *skel;
    int err;
    /* Parse command line arguments */
    if (argc > 1)
    {
        err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
        if (err)
            return err;
    }
    // libbpf_set_print(libbpf_print_fn);

    signal(SIGUSR1, sig_handler);

    skel = signalkill_bpf__open();
    if (!skel)
    {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }
    /* Parameterize BPF code */
    err = signalkill_bpf__load(skel);
    if (err)
    {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }
    /*update map*/
    int pid = getpid();
    int key = 0;
    err = bpf_map_update_elem(bpf_map__fd(skel->maps.pid_map), &key, &pid, BPF_ANY);
    if (err)
    {
        fprintf(stderr, "Failed to update PID map\n");
        goto cleanup;
    }

    /* Attach tracepoint handler */
    err = signalkill_bpf__attach(skel);
    if (err)
    {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }
    /* Set up ring buffer polling */
    rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
    if (!rb)
    {
        err = -1;
        fprintf(stderr, "Failed to create ring buffer(packet)\n");
        goto cleanup;
    }
    // 初始化任务系统
    init();

    // 创建多个任务
    task_start("task1", 31, test, "argA ");
    for (int i = 0; i < 25; i++)
    {
        task_start("abc", 31, test1, "a ");
    }
    /* Process events */
    while (!exiting)
    {
        err = ring_buffer__poll(rb, 100 /* timeout, ms */);
        sleep(1);
        /* Ctrl-C will cause -EINTR */
        if (err == -EINTR)
        {
            err = 0;
            break;
        }
        if (err < 0)
        {
            printf("Error polling perf buffer: %d\n", err);
            break;
        }
    }
cleanup:
    ring_buffer__free(rb);
    signalkill_bpf__destroy(skel);
    return err < 0 ? -err : 0;
}