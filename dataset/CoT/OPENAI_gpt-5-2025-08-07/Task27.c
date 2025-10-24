#include <stdio.h>
#include <stddef.h>
#include <assert.h>

int find_outlier(const int* arr, size_t n) {
    assert(arr != NULL);
    assert(n >= 3);
    int oddCount = 0;
    size_t limit = n < 3 ? n : 3;
    for (size_t i = 0; i < limit; ++i) {
        if ((arr[i] & 1) != 0) ++oddCount;
    }
    int majorityOdd = (oddCount >= 2) ? 1 : 0;
    for (size_t i = 0; i < n; ++i) {
        int vOdd = ((arr[i] & 1) != 0) ? 1 : 0;
        if (vOdd != majorityOdd) return arr[i];
    }
    assert(0 && "No outlier found");
    return 0;
}

int main(void) {
    int t1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    int t2[] = {160, 3, 1719, 19, 11, 13, -21};
    int t3[] = {3, 5, 7, 9, 2, 11, 13};
    int t4[] = {2, 4, 6, 8, 10, 3, 12};
    int t5[] = {-2, -4, -6, -8, -11, -12};

    printf("%d\n", find_outlier(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%d\n", find_outlier(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%d\n", find_outlier(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%d\n", find_outlier(t4, sizeof(t4)/sizeof(t4[0])));
    printf("%d\n", find_outlier(t5, sizeof(t5)/sizeof(t5[0])));

    return 0;
}