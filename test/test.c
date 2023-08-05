#include <stdio.h>

extern double fib(double);

int main()
{
    printf("fib 50 = %f\n", fib(50.0));
    // for (int n = 0; n < 10; n++) {
    //     printf("fac %d = %f\n", n, factorial((float) n));
    // }
}