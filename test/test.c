#include <stdio.h>

extern double fib(double);

int main()
{
    for (int n = 0; n < 10; n++) {
        printf("fib %d = %f\n", n, fib((float) n));
    }
}