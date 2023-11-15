#include <stdio.h>

extern double entry();

int main() {
    double val = entry();
    printf("%f\n", val);
    return 0;
}