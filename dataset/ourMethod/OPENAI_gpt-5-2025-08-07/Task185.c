#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>

// Returns the median of two sorted arrays in O(log(m+n)) time.
// If both arrays are empty or inputs invalid, returns NAN.
double findMedianSortedArrays(const int* a, size_t m, const int* b, size_t n) {
    // Validate inputs: if length > 0 then pointer must be non-null
    if ((m > 0 && a == NULL) || (n > 0 && b == NULL)) {
        return NAN;
    }
    if (m == 0 && n == 0) {
        return NAN;
    }

    // Ensure A is the smaller array
    const int* A = a;
    const int* B = b;
    size_t mlen = m, nlen = n;
    if (mlen > nlen) {
        A = b; B = a;
        mlen = n; nlen = m;
    }

    int m_i = (int)mlen;
    int n_i = (int)nlen;

    int low = 0, high = m_i;
    int half = (m_i + n_i + 1) / 2;

    while (low <= high) {
        int i = (low + high) / 2;
        int j = half - i;

        int Aleft  = (i == 0)     ? INT_MIN : A[i - 1];
        int Aright = (i == m_i)    ? INT_MAX : A[i];
        int Bleft  = (j == 0)     ? INT_MIN : B[j - 1];
        int Bright = (j == n_i)    ? INT_MAX : B[j];

        if (Aleft <= Bright && Bleft <= Aright) {
            if (((m_i + n_i) & 1) == 0) {
                long long leftMax = (Aleft > Bleft) ? Aleft : Bleft;
                long long rightMin = (Aright < Bright) ? Aright : Bright;
                return (leftMax + rightMin) / 2.0;
            } else {
                int leftMax = (Aleft > Bleft) ? Aleft : Bleft;
                return (double)leftMax;
            }
        } else if (Aleft > Bright) {
            high = i - 1;
        } else {
            low = i + 1;
        }
    }

    return NAN; // Should not reach here if inputs are sorted
}

static void run_test(const int* a, size_t m, const int* b, size_t n) {
    double res = findMedianSortedArrays(a, m, b, n);
    if (isnan(res)) {
        printf("Error\n");
    } else {
        printf("%.5f\n", res);
    }
}

int main(void) {
    // 5 Test cases
    int a1[] = {1, 3};         int b1[] = {2};
    run_test(a1, 2, b1, 1);    // 2.00000

    int a2[] = {1, 2};         int b2[] = {3, 4};
    run_test(a2, 2, b2, 2);    // 2.50000

    int a3[] = {};             int b3[] = {1};
    run_test(a3, 0, b3, 1);    // 1.00000

    int a4[] = {0, 0};         int b4[] = {0, 0};
    run_test(a4, 2, b4, 2);    // 0.00000

    int a5[] = {-1000000, -5, 0, 7};
    int b5[] = {-3, 2, 2, 1000000};
    run_test(a5, 4, b5, 4);    // 1.00000

    return 0;
}