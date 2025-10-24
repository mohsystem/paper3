#include <stdio.h>
#include <limits.h>

double findMedianSortedArrays(const int* a, int m, const int* b, int n) {
    // Ensure 'a' is the smaller array
    if (m > n) {
        const int* tmpP = a; a = b; b = tmpP;
        int tmpL = m; m = n; n = tmpL;
    }

    int totalLeft = (m + n + 1) / 2;
    int lo = 0, hi = m;

    while (lo <= hi) {
        int i = lo + (hi - lo) / 2;
        int j = totalLeft - i;

        int leftA = (i == 0) ? INT_MIN : a[i - 1];
        int rightA = (i == m) ? INT_MAX : a[i];
        int leftB = (j == 0) ? INT_MIN : b[j - 1];
        int rightB = (j == n) ? INT_MAX : b[j];

        if (leftA <= rightB && leftB <= rightA) {
            if (((m + n) & 1) == 1) {
                return (double)(leftA > leftB ? leftA : leftB);
            } else {
                int maxLeft = leftA > leftB ? leftA : leftB;
                int minRight = rightA < rightB ? rightA : rightB;
                return ((double)maxLeft + (double)minRight) / 2.0;
            }
        } else if (leftA > rightB) {
            hi = i - 1;
        } else {
            lo = i + 1;
        }
    }
    // Should not reach here if inputs meet constraints
    return 0.0;
}

int main(void) {
    int a1[] = {1,3}; int b1[] = {2};
    int a2[] = {1,2}; int b2[] = {3,4};
    int a3[] = {};     int b3[] = {1};
    int a4[] = {0,0};  int b4[] = {0,0};
    int a5[] = {2};    int b5[] = {};

    printf("%.5f\n", findMedianSortedArrays(a1, 2, b1, 1));
    printf("%.5f\n", findMedianSortedArrays(a2, 2, b2, 2));
    printf("%.5f\n", findMedianSortedArrays(a3, 0, b3, 1));
    printf("%.5f\n", findMedianSortedArrays(a4, 2, b4, 2));
    printf("%.5f\n", findMedianSortedArrays(a5, 1, b5, 0));
    return 0;
}