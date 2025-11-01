#include <stdio.h>
#include <stdlib.h>

int run_case(unsigned int seed) {
    srand(seed);
    int a = rand() % 9000 + 1001;
    int b = rand() % 9000 + 1001;
    int c = rand() % 9000 + 1001;
    int sum = a + b + c;
    printf("a : %d b : %d c : %d\n", a, b, c);
    printf("sum: %d sum2: %d\n", sum, sum);
    return sum;
}

int main() {
    run_case(1U);
    run_case(2U);
    run_case(3U);
    run_case(4U);
    run_case(5U);
    return 0;
}