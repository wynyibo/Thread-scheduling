#include <iostream>
#include <sys/time.h>
#include <cmath>
#define N 100000000
using namespace std;

int main()
{
    timeval start, end;
    int i;
    double x, pi, sum = 0;
    double step = 1.0 / N;
    gettimeofday(&start, NULL);
#pragma omp parallel for reduction(+ : sum) private(x)
// g++ -O1 -fopenmp simd.cpp -o simd
// #pragma omp simd reduction(+:sum) private(x)
// #pragma omp parallel for simd reduction(+:sum) private(x)
// g++ -O1 -fopenmp -fopt-info-vec-optimized -ftree-vectorizer-verbose=2 simd.cpp -o simd
    for (i = 0; i < N; i++)
    {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    gettimeofday(&end, NULL);
    pi = sum * step;
    int time_used = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    cout << "time_used=" << time_used << endl;
    cout << "PI=" << pi << endl;
    return 0;
}