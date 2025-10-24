#include <stdio.h>
#include <limits.h>

double findMedianSortedArrays(const int* nums1, int m, const int* nums2, int n) {
    const int* A = nums1;
    const int* B = nums2;
    int mlen = m, nlen = n;
    if (mlen > nlen) {
        A = nums2; B = nums1;
        int tmp = mlen; mlen = nlen; nlen = tmp;
    }
    int imin = 0, imax = mlen, half = (mlen + nlen + 1) / 2;
    while (imin <= imax) {
        int i = (imin + imax) / 2;
        int j = half - i;
        if (i < mlen && j > 0 && B[j - 1] > A[i]) {
            imin = i + 1;
        } else if (i > 0 && j < nlen && A[i - 1] > B[j]) {
            imax = i - 1;
        } else {
            int maxLeft;
            if (i == 0) maxLeft = B[j - 1];
            else if (j == 0) maxLeft = A[i - 1];
            else maxLeft = (A[i - 1] > B[j - 1] ? A[i - 1] : B[j - 1]);

            if (((mlen + nlen) & 1) == 1) return (double)maxLeft;

            int minRight;
            if (i == mlen) minRight = B[j];
            else if (j == nlen) minRight = A[i];
            else minRight = (A[i] < B[j] ? A[i] : B[j]);

            return ((double)maxLeft + (double)minRight) / 2.0;
        }
    }
    return 0.0;
}

int main() {
    int a1[] = {1, 3}; int b1[] = {2};
    int a2[] = {1, 2}; int b2[] = {3, 4};
    const int* a3 = NULL; int b3[] = {1};
    int a4[] = {0, 0}; int b4[] = {0, 0};
    int a5[] = {2}; const int* b5 = NULL;

    printf("%.5f\n", findMedianSortedArrays(a1, 2, b1, 1));
    printf("%.5f\n", findMedianSortedArrays(a2, 2, b2, 2));
    printf("%.5f\n", findMedianSortedArrays(a3, 0, b3, 1));
    printf("%.5f\n", findMedianSortedArrays(a4, 2, b4, 2));
    printf("%.5f\n", findMedianSortedArrays(a5, 1, b5, 0));
    return 0;
}