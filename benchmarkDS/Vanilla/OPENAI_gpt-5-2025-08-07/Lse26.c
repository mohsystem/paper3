#include <stdio.h>

int getMonthlySales(int i, const int* sales) {
    return sales[i];
}

int totalFirstQuarterSales(const int* sales) {
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += getMonthlySales(i, sales);
    }
    return sum;
}

int main() {
    int test1[] = {100, 200, 300};
    int test2[] = {0, 0, 0, 0};
    int test3[] = {5, 10, 15, 20};
    int test4[] = {1000, -50, 25, 0};
    int test5[] = {7, 8, 9, 10, 11};

    printf("Test 1: %d\n", totalFirstQuarterSales(test1));
    printf("Test 2: %d\n", totalFirstQuarterSales(test2));
    printf("Test 3: %d\n", totalFirstQuarterSales(test3));
    printf("Test 4: %d\n", totalFirstQuarterSales(test4));
    printf("Test 5: %d\n", totalFirstQuarterSales(test5));

    return 0;
}