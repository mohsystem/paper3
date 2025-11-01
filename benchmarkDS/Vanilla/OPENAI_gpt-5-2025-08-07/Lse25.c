#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getMonthlySales() {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
    return rand() % 100001;
}

void generateYearlySales(int monthly[12]) {
    for (int i = 0; i < 12; ++i) {
        monthly[i] = getMonthlySales();
    }
}

void quarterlySums(const int monthly[12], long long quarters[4]) {
    for (int i = 0; i < 4; ++i) quarters[i] = 0;
    for (int i = 0; i < 12; ++i) {
        quarters[i / 3] += monthly[i];
    }
}

void printQuarterly(const long long quarters[4], int caseNo) {
    printf("Test case %d:\n", caseNo);
    for (int i = 0; i < 4; ++i) {
        printf("Q%d: %lld\n", i + 1, quarters[i]);
    }
}

int main() {
    for (int t = 1; t <= 5; ++t) {
        int monthly[12];
        long long q[4];
        generateYearlySales(monthly);
        quarterlySums(monthly, q);
        printQuarterly(q, t);
    }
    return 0;
}