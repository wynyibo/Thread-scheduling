#include <stdio.h>
int main(int argc, char *argv[])
{
    const int N = 1000;
    double A[N];
    for (int i = 0; i < N; ++i)
        A[i] = i * i;
    return 0;
}