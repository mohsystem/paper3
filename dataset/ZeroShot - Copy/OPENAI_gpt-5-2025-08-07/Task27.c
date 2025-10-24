#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

long long findOutlier(const long long* arr, size_t n) {
    if (arr == NULL || n < 3) {
        errno = EINVAL;
        return 0;
    }
    int evenCount = 0;
    for (size_t i = 0; i < 3; ++i) {
        if ( (arr[i] & 1LL) == 0LL ) {
            ++evenCount;
        }
    }
    bool majorityEven = evenCount >= 2;
    for (size_t i = 0; i < n; ++i) {
        bool isEven = ( (arr[i] & 1LL) == 0LL );
        if (isEven != majorityEven) {
            return arr[i];
        }
    }
    errno = ENOENT;
    return 0;
}

int main(void) {
    long long a1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    long long a2[] = {160, 3, 1719, 19, 11, 13, -21};
    long long a3[] = {-2, -4, -6, -8, -10, -3, -12};
    long long a4[] = {1, 3, 5, 7, 8, 9, 11, 13};
    long long a5[] = {42, 6, 8, 10, 12, 14, 16, 7};

    printf("%lld\n", findOutlier(a1, sizeof(a1)/sizeof(a1[0])));
    printf("%lld\n", findOutlier(a2, sizeof(a2)/sizeof(a2[0])));
    printf("%lld\n", findOutlier(a3, sizeof(a3)/sizeof(a3[0])));
    printf("%lld\n", findOutlier(a4, sizeof(a4)/sizeof(a4[0])));
    printf("%lld\n", findOutlier(a5, sizeof(a5)/sizeof(a5[0])));

    return 0;
}