#include <stdio.h>

extern double entry();

int main() {
    double val = entry();
    printf("%f", val);
    return 0;
}