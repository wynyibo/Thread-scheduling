#include <omp.h>
#include <asm/types.h>
#include <stdio.h>
void x_calculation(void);
void y_calculation(void);
void z_calculation(void);

int main(int argc, char *argv[])
{
#pragma omp parallel
    {
#pragma omp sections // nowait
        {
#pragma omp section
            x_calculation(); // independent computation
#pragma omp section
            y_calculation(); // independent computation
#pragma omp section
            z_calculation(); // independent computation
        }                    // team 中的其他线程在此等待
                             // 使用#pragma omp sections nowait让它们在不同步的情况下继续
    }
    return 0;
}

void x_calculation(void) {
    float term,result=1;
    int n;
    for(n=2;n<=10000;n+=2){
        term=(float)(n*n)/((n-1)*(n+1));
        result*=term;
    }
    printf("pi的值为：%f\n", 2*result);
}

void y_calculation(void) {
    float e=1.0,n=1.0;
    int i=1;
    while(1/n>1e-10)
    {
        e+=1/n;
        i++;
        n=i*n;
    }
    printf("e的值是：%f\n",e);
}

double myabs(double num1)
{
    return((num1 > 0) ? num1 : -num1);
}
void z_calculation(void)
{
    double num2 = 3.14/6;
    int i = 1, negation = 1;//取反
    double sum;
    double index = num2;//指数
    double Factorial = 1;//阶乘
    double TaylorExpansion = num2;//泰勒展开式求和
    do
    {
        Factorial = Factorial * ((__int64_t)i + 1) * ((__int64_t)i + 2);//求阶乘
        index *= num2 * num2;//求num2的次方
        negation = -negation;//每次循环取反
        sum = index / Factorial * negation;
        TaylorExpansion += sum;
        i += 2;
    } while (myabs(sum) > 1e-15);
    printf("sin(π/6)的值是：%lf\n",TaylorExpansion);
}