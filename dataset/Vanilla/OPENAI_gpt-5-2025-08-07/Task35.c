#include <stdio.h>

int countPeople(const int stops[][2], int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += stops[i][0] - stops[i][1];
    }
    return total;
}

int main() {
    int test1[][2] = { {10,0}, {3,5}, {5,8} }; // Expected: 5
    int test2[][2] = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; // Expected: 17
    int test3[][2] = { {0,0} }; // Expected: 0
    int test4[][2] = { {5,0}, {5,5}, {10,10}, {1,1} }; // Expected: 0
    int test5[][2] = { {50,0}, {30,10}, {5,5}, {40,20}, {0,0}, {1,1} }; // Expected: 90

    printf("%d\n", countPeople(test1, sizeof(test1)/sizeof(test1[0])));
    printf("%d\n", countPeople(test2, sizeof(test2)/sizeof(test2[0])));
    printf("%d\n", countPeople(test3, sizeof(test3)/sizeof(test3[0])));
    printf("%d\n", countPeople(test4, sizeof(test4)/sizeof(test4[0])));
    printf("%d\n", countPeople(test5, sizeof(test5)/sizeof(test5[0])));

    return 0;
}