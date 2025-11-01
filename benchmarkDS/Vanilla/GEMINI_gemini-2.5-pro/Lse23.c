#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateAndSumRandoms() {
    int a = rand() % 8999 + 1001;
    int b = rand() % 8999 + 1001;
    int c = rand() % 8999 + 1001;

    printf("a : %d\n", a);
    printf("b : %d\n", b);
    printf("c : %d\n", c);

    int sum = a + b + c;
    printf("sum: %d\n", sum);
    printf("sum2: %d\n", sum);
}

int main() {
    srand(time(NULL));
    
    for (int i = 0; i < 5; ++i) {
        generateAndSumRandoms();
        if (i < 4) {
            printf("\n");
        }
    }
    
    return 0;
}