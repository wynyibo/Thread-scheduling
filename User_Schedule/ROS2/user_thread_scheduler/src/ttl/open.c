#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <linux/netlink.h>
#include <fcntl.h>
#include <omp.h>

#define RAND_MINS 0
#define RAND_MAXS 2
#define NETLINK_USER 31

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

// 结构体来存储线程的优先级
typedef struct
{
    int priority;
    char *message;
} thread_args;

// 定义神经网络的结构
typedef struct
{
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

double sigmoid(double x)
{
    return 1 / (1 + exp(-x));
}

// 初始化权重和偏置
void init_net(NeuralNet *net)
{
    net->W1 = (double *)malloc(net->input_size * net->hidden_size * sizeof(double));
    net->b1 = (double *)malloc(net->hidden_size * sizeof(double));
    net->W2 = (double *)malloc(net->hidden_size * net->output_size * sizeof(double));
    net->b2 = (double *)malloc(net->output_size * sizeof(double));
    srand(time(NULL));
    for (int i = 0; i < net->input_size * net->hidden_size; i++)
    {
        net->W1[i] = (double)rand() / RAND_MAXS;
    }
    for (int i = 0; i < net->hidden_size * net->output_size; i++)
    {
        net->W2[i] = (double)rand() / RAND_MAXS;
    }
    for (int i = 0; i < net->hidden_size; i++)
    {
        net->b1[i] = (double)rand() / RAND_MAXS;
    }
    for (int i = 0; i < net->output_size; i++)
    {
        net->b2[i] = (double)rand() / RAND_MAXS;
    }
}

// 前向传播算法
void forward(NeuralNet *net, double *input, double *output)
{
    double *h1 = (double *)malloc(net->hidden_size * sizeof(double));
    double *h2 = (double *)malloc(net->output_size * sizeof(double));
    for (int i = 0; i < net->hidden_size; i++)
    {
        h1[i] = 0;
        for (int j = 0; j < net->input_size; j++)
        {
            h1[i] += input[j] * net->W1[j * net->hidden_size + i];
        }
        h1[i] += net->b1[i];
        h1[i] = tanh(h1[i]);
    }
    for (int i = 0; i < net->output_size; i++)
    {
        h2[i] = 0;
        for (int j = 0; j < net->hidden_size; j++)
        {
            h2[i] += h1[j] * net->W2[j * net->output_size + i];
        }
        h2[i] += net->b2[i];
    }
    for (int i = 0; i < net->output_size; i++)
    {
        output[i] = sigmoid(h2[i]);
    }
    free(h1);
    free(h2);
}

// 反向传播算法
void backward(NeuralNet *net, double *input, double *target, double learning_rate)
{
    double *h1 = (double *)malloc(net->hidden_size * sizeof(double));
    double *h2 = (double *)malloc(net->output_size * sizeof(double));
    double *delta1 = (double *)malloc(net->hidden_size * sizeof(double));
    double *delta2 = (double *)malloc(net->output_size * sizeof(double));
    for (int i = 0; i < net->hidden_size; i++)
    {
        h1[i] = 0;
        for (int j = 0; j < net->input_size; j++)
        {
            h1[i] += input[j] * net->W1[j * net->hidden_size + i];
        }
        h1[i] += net->b1[i];
        h1[i] = tanh(h1[i]);
    }
    for (int i = 0; i < net->output_size; i++)
    {
        h2[i] = 0;
        for (int j = 0; j < net->hidden_size; j++)
        {
            h2[i] += h1[j] * net->W2[j * net->output_size + i];
        }
        h2[i] += net->b2[i];
    }
    for (int i = 0; i < net->output_size; i++)
    {
        h2[i] = sigmoid(h2[i]);
        delta2[i] = h2[i] * (1 - h2[i]) * (target[i] - h2[i]);
    }
    for (int i = 0; i < net->hidden_size; i++)
    {
        delta1[i] = 0;
        for (int j = 0; j < net->output_size; j++)
        {
            delta1[i] += delta2[j] * net->W2[i * net->output_size + j];
        }
        delta1[i] *= (1 - h1[i]) * (1 + h1[i]);
    }
    for (int i = 0; i < net->hidden_size; i++)
    {
        for (int j = 0; j < net->input_size; j++)
        {
            net->W1[j * net->hidden_size + i] += learning_rate * delta1[i] * input[j];
        }
        net->b1[i] += learning_rate * delta1[i];
    }
    for (int i = 0; i < net->output_size; i++)
    {
        for (int j = 0; j < net->hidden_size; j++)
        {
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
void train(NeuralNet *net, double *inputs, double *targets, int num_epochs, int num_inputs, double learning_rate)
{
    for (int i = 0; i < num_epochs; i++)
    {
        for (int j = 0; j < num_inputs; j++)
        {
            double *input = &inputs[j * net->input_size];
            double *target = &targets[j * net->output_size];
            backward(net, input, target, learning_rate);
        }
    }
}

// 使用神经网络进行预测
void predict(NeuralNet *net, double *input, int *output)
{
    double temp_output[1]; // 临时存储浮点结果
    forward(net, input, temp_output);
    output[0] = (int)(temp_output[0] * 100000); // 将浮点数乘以100转为整数形式
}
// long long total_io_bytes = 0;
// int elapsed_time;
void my_task_func()
{
    pid_t pid = getpid();
    printf("Task started\n");
    fflush(stdout); // 确保立即打印

    double io_time = 0.0;

    clock_t start_time = clock();
    NeuralNet net;
    net.input_size = 2;
    net.hidden_size = 3;
    net.output_size = 1;
    init_net(&net);

    double inputs[8];
    double targets[4];

    for (int i = 0; i < 8; i++)
    {
        inputs[i] = RAND_MINS + (RAND_MAXS - RAND_MINS) * ((double)rand() / RAND_MAXS);
    }
    for (int i = 0; i < 4; i++)
    {
        targets[i] = RAND_MINS + (RAND_MAXS - RAND_MINS) * ((double)rand() / RAND_MAXS);
    }

    // 执行I/O操作，如发送 Netlink 消息
    ssize_t bytes_sent = sendmsg(sock_fd, &msg, 0);
    // 在进行I/O操作之后记录结束时间
    // 在进行I/O操作之前记录开始时间
    
    // printf("I/O Throughput: %lf bytes/second\n", io_throughput);

    // 继续神经网络操作
    train(&net, inputs, targets, 1000, 8, 0.1);
    int output[1];
    predict(&net, inputs, output);

    clock_t end_time = clock();
    int elapsed_time = (double)(end_time - start_time);
    // 计算I/O操作耗时（以秒为单位）

    // int io_throughput = total_io_bytes / elapsed_time;
    printf("Bytes sent: %zd\n", bytes_sent);
    // printf("I/O=====:%d",total_io_bytes);
    printf("elapsed_time==%d", elapsed_time);
    // clock_t end_time = clock();
    // double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    // printf("Execution time: %lf seconds\n", elapsed_time);

    // 将执行时间写入到文件中
    FILE *file = fopen("data0.txt", "a+");
    if (file != NULL)
    {
        fprintf(file, "%d %d\n", bytes_sent,elapsed_time);
        fclose(file);
    }
    else
    {
        perror("fopen");
    }

    free(net.W1);
    free(net.b1);
    free(net.W2);
    free(net.b2);

    printf("Task completed\n");
    fflush(stdout);
}
int io_throughput;

int main()
{
        const int num_threads = 50; // 线程数量

    printf("Starting main...\n");

    // 使用 OpenMP 进行并行化
    #pragma omp parallel num_threads(num_threads + 1)
    {
        my_task_func();
    }

    // 主线程持续运行
    while (1) {
        sleep(1);
        printf("%d\n",getpid());
    }

    return 0;
}
