#ifndef __SIGNALKILL_H
#define __SIGNALKILL_H

#define TASK_COMM_LEN 16
struct event {
    int pid;
    char comm[TASK_COMM_LEN];
    bool success;
};

#endif