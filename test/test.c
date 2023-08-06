#include <stdio.h>

extern double mod(double, double);

int main()
{
    for (int n = 0; n < 2; n++) {
        printf("2 %% %d = %f\n", n, mod(2.0, (float) n));
    }
}