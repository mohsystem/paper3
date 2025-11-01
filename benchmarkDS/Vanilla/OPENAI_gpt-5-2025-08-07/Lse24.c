#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int a;
    int b;
    int diff;
} Result;

Result generate_and_diff(int min, int max) {
    Result r;
    r.a = (rand() % (max - min + 1)) + min;
    r.b = (rand() % (max - min + 1)) + min;
    r.diff = r.a - r.b;
    return r;
}

int main() {
    srand((unsigned int)time(NULL));
    for (int i = 1; i <= 5; i++) {
        Result r = generate_and_diff(10, 1000);
        printf("Test %d: A=%d, B=%d, Difference=%d\n", i, r.a, r.b, r.diff);
    }
    return 0;
}