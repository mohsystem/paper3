#include <stdio.h>
#include <stdlib.h>

static unsigned long long TASK198_SEED = 123456789ULL;

void Task198_resetSeed(unsigned long long s) {
    TASK198_SEED = s;
}

int Task198_nextRand() {
    TASK198_SEED = (TASK198_SEED * 1103515245ULL + 12345ULL) & 0x7fffffffULL;
    return (int)TASK198_SEED;
}

// Given API
int rand7() {
    return (Task198_nextRand() % 7) + 1;
}

// Use only rand7()
int rand10() {
    while (1) {
        int a = rand7();
        int b = rand7();
        int num = (a - 1) * 7 + b; // 1..49
        if (num <= 40) return ((num - 1) % 10) + 1;
    }
}

int* generate(int n) {
    int* arr = (int*)malloc(sizeof(int) * n);
    if (!arr) return NULL;
    for (int i = 0; i < n; ++i) {
        arr[i] = rand10();
    }
    return arr;
}

void printArray(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    int tests[5] = {1, 2, 3, 10, 20};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        Task198_resetSeed(123456789ULL);
        int* res = generate(n);
        if (res) {
            printArray(res, n);
            free(res);
        }
    }
    return 0;
}