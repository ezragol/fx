#include <stdio.h>

extern double mod(double, double);

int main()
{
    int n = 50;
    int m = 3;
    printf("%d mod %d = %f\n", n, m, mod((float) n, (float) m));
}