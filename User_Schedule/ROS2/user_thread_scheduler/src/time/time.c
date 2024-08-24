#include "task.h"
#include <math.h>
#include "console.h"
#include "init.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <fcntl.h>

#define RAND_MINS 0
#define RAND_MAXS 2
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

// 定义神经网络的结构
typedef struct {
    int input_size;
    int hidden_size;
    int output_size;
    double *W1;
    double *b1;
    double *W2;
    double *b2;
} NeuralNet;

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
    
    return abs(random_value) % 32;
}

double sigmoid(double x) {
    return 1 / (1 + exp(-x));
}
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}
// Xavier 初始化
void init_net(NeuralNet *net) {
    double limit_W1 = sqrt(6.0 / (net->input_size + net->hidden_size));
    double limit_W2 = sqrt(6.0 / (net->hidden_size + net->output_size));
    
    net->W1 = (double*) malloc(net->input_size * net->hidden_size * sizeof(double));
    net->b1 = (double*) malloc(net->hidden_size * sizeof(double));
    net->W2 = (double*) malloc(net->hidden_size * net->output_size * sizeof(double));
    net->b2 = (double*) malloc(net->output_size * sizeof(double));

    if (!net->W1 || !net->b1 || !net->W2 || !net->b2) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    for (int i = 0; i < net->input_size * net->hidden_size; i++) {
        net->W1[i] = (2.0 * rand() / RAND_MAX - 1.0) * limit_W1;
    }
    for (int i = 0; i < net->hidden_size * net->output_size; i++) {
        net->W2[i] = (2.0 * rand() / RAND_MAX - 1.0) * limit_W2;
    }

    memset(net->b1, 0, net->hidden_size * sizeof(double));
    memset(net->b2, 0, net->output_size * sizeof(double));
}

// 前向传播算法
void forward(NeuralNet *net, double *input, double *output) {
    double *h1 = (double*) malloc(net->hidden_size * sizeof(double));
    double *h2 = (double*) malloc(net->output_size * sizeof(double));
    for (int i = 0; i < net->hidden_size; i++) {
        h1[i] = 0;
        for (int j = 0; j < net->input_size; j++) {
            h1[i] += input[j] * net->W1[j * net->hidden_size + i];
        }
        h1[i] += net->b1[i];
        h1[i] = tanh(h1[i]);
    }
    for (int i = 0; i < net->output_size; i++) {
        h2[i] = 0;
        for (int j = 0; j < net->hidden_size; j++) {
            h2[i] += h1[j] * net->W2[j * net->output_size + i];
        }
        h2[i] += net->b2[i];
    }
    for (int i = 0; i < net->output_size; i++) {
        output[i] = sigmoid(h2[i]);
    }
    free(h1);
    free(h2);
}

// 反向传播算法
void backward(NeuralNet *net, double *input, double *target, double learning_rate) {
    double *h1 = (double*) malloc(net->hidden_size * sizeof(double));
    double *h2 = (double*) malloc(net->output_size * sizeof(double));
    double *delta1 = (double*) malloc(net->hidden_size * sizeof(double));
    double *delta2 = (double*) malloc(net->output_size * sizeof(double));
    for (int i = 0; i < net->hidden_size; i++) {
        h1[i] = 0;
        for (int j = 0; j < net->input_size; j++) {
            h1[i] += input[j] * net->W1[j * net->hidden_size + i];
        }
        h1[i] += net->b1[i];
        h1[i] = tanh(h1[i]);
    }
    for (int i = 0; i < net->output_size; i++) {
        h2[i] = 0;
        for (int j = 0; j < net->hidden_size; j++) {
            h2[i] += h1[j] * net->W2[j * net->output_size + i];
        }
        h2[i] += net->b2[i];
    }
    for (int i = 0; i < net->output_size; i++) {
        h2[i] = sigmoid(h2[i]);
        delta2[i] = h2[i] * (1 - h2[i]) * (target[i] - h2[i]);
    }
    for (int i = 0; i < net->hidden_size; i++) {
        delta1[i] = 0;
        for (int j = 0; j < net->output_size; j++) {
            delta1[i] += delta2[j] * net->W2[i * net->output_size + j];
        }
        delta1[i] *= (1 - h1[i]) * (1 + h1[i]);
    }
    for (int i = 0; i < net->hidden_size; i++) {
        for (int j = 0; j < net->input_size; j++) {
            net->W1[j * net->hidden_size + i] += learning_rate * delta1[i] * input[j];
        }
        net->b1[i] += learning_rate * delta1[i];
    }
    for (int i = 0; i < net->output_size; i++) {
        for (int j = 0; j < net->hidden_size; j++) {
            net->W2[j * net->output_size + i] += learning_rate * delta2[i] * h1[j];
        }
        net->b2[i] += learning_rate * delta2[i];
    }
    free(h1);
    free(h2);
    free(delta1);
    free(delta2);
}

// 训练神经网络
void train(NeuralNet *net, double *inputs, double *targets, int num_epochs, int num_inputs, double learning_rate) {
    for (int i = 0; i < num_epochs; i++) {
        for (int j = 0; j < num_inputs; j++) {
            double *input = &inputs[j * net->input_size];
            double *target = &targets[j * net->output_size];
            backward(net, input, target, learning_rate);
        }
    }
}

// 使用神经网络进行预测
void predict(NeuralNet *net, double *input, int *output) {
    double temp_output[1];
    forward(net, input, temp_output);
    output[0] = (int)(temp_output[0] * 100000);
}


void my_task_func(void* arg) {
    pid_t pid = getpid();
    printf("Task started\n");
    // while(1){
    //      sleep(1);
    //     console_put_str(arg);
    //     printf("%d\n",pid);
    // }
    fflush(stdout); // 确保立即打印

    clock_t start_time = clock();

    NeuralNet net;
    net.input_size = 2;
    net.hidden_size = 3;
    net.output_size = 1;
    init_net(&net);

    double inputs[8];
    double targets[4];
    
    for (int i = 0; i < 8; i++) {
        inputs[i] = RAND_MINS + (RAND_MAXS - RAND_MINS) * ((double)rand() / RAND_MAXS);
    }
    for (int i = 0; i < 4; i++) {
        targets[i] = RAND_MINS + (RAND_MAXS - RAND_MINS) * ((double)rand() / RAND_MAXS);
    }

    train(&net, inputs, targets, 1000, 8, 0.1);
    int output[1];
    predict(&net, inputs, output);
    // print_prediction(output);
    clock_t end_time = clock();
    int elapsed_time =(double)(end_time - start_time);
    printf("Execution time: %d seconds\n", elapsed_time);
        // 将执行时间写入到文件中
    FILE *file = fopen("time.txt", "a+");
    if (file != NULL) {
        fprintf(file, "%d\n", elapsed_time);
        fclose(file);
    } else {
        perror("fopen");
    }

    free(net.W1);
    free(net.b1);
    free(net.W2);
    free(net.b2);

    printf("Task completed\n");
    fflush(stdout); // 确保立即打印
    // }
}
int main() {
    printf("Starting main...\n");
    int priority = generate_random_priority();
    pid_t pid = getpid();

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }
    
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(pid_t) + sizeof(int)));
    if (nlh == NULL) {
        perror("malloc");
        close(sock_fd);
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(sizeof(pid_t) + sizeof(int)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(pid_t) + sizeof(int));
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    int *data = (int *)NLMSG_DATA(nlh);
    data[0] = pid;
    data[1] = priority;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(sock_fd, &msg, 0);
    printf("Sent PID %d and priority %d to kernel module\n", pid, priority);

    fflush(stdout);

    init(); // Initialize task system
    const int interfering_thread_num = 25;

    for (int i = 0; i < interfering_thread_num; i++) {
        thread_args *args = malloc(sizeof(thread_args));
        if (args == NULL) {
            perror("malloc");
            return -1;
        }
        args->priority = generate_random_priority();
        args->message = "thread ";
        task_start("interfering_task", args->priority, my_task_func, args);
    }
    printf("\n");

    thread_args *args = malloc(sizeof(thread_args));
    if (args == NULL) {
        perror("malloc");
        return -1;
    }
    args->priority = generate_random_priority();
    args->message = "argA ";
    task_start("main_task", args->priority, my_task_func, args);
    printf("\n");
    task_wait_all();

    while (1) {
        sleep(1);
        console_put_str("=================TASK===============\n");
    }

    free(nlh);
    close(sock_fd);
    fflush(stdout);

    return 0;
}
