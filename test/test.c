#include <stdio.h>

extern double fib(double);

int main()
{
    int n = 10;
    printf("fib %d = %f\n", n, fib((float) n));
}