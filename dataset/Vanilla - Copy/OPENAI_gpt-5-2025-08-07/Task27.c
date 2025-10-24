#include <stdio.h>

int findOutlier(const int* arr, int n) {
    int evenCount = 0;
    int limit = n < 3 ? n : 3;
    for (int i = 0; i < limit; ++i) {
        if ((arr[i] & 1) == 0) evenCount++;
    }
    int majorityEven = evenCount >= 2;
    for (int i = 0; i < n; ++i) {
        if (majorityEven && (arr[i] & 1)) return arr[i];
        if (!majorityEven && ((arr[i] & 1) == 0)) return arr[i];
    }
    return 0; // Should not happen for valid inputs
}

int main() {
    int t1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    int t2[] = {160, 3, 1719, 19, 11, 13, -21};
    int t3[] = {1, 1, 2};
    int t4[] = {2, 6, 8, 10, 3};
    int t5[] = {-2, -4, -6, -7, -8};

    printf("Test 1 outlier: %d\n", findOutlier(t1, sizeof(t1)/sizeof(t1[0])));
    printf("Test 2 outlier: %d\n", findOutlier(t2, sizeof(t2)/sizeof(t2[0])));
    printf("Test 3 outlier: %d\n", findOutlier(t3, sizeof(t3)/sizeof(t3[0])));
    printf("Test 4 outlier: %d\n", findOutlier(t4, sizeof(t4)/sizeof(t4[0])));
    printf("Test 5 outlier: %d\n", findOutlier(t5, sizeof(t5)/sizeof(t5[0])));
    return 0;
}