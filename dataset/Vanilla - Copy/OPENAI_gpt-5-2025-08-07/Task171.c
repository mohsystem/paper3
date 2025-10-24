#include <stdio.h>
#include <stdlib.h>

static long long sortCount(int* a, int l, int r, int* temp) {
    if (l >= r) return 0LL;
    int m = l + (r - l) / 2;
    long long cnt = sortCount(a, l, m, temp) + sortCount(a, m + 1, r, temp);

    int j = m + 1;
    for (int i = l; i <= m; ++i) {
        while (j <= r && (long long)a[i] > 2LL * (long long)a[j]) j++;
        cnt += (j - (m + 1));
    }

    int i = l; j = m + 1; int k = l;
    while (i <= m && j <= r) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= m) temp[k++] = a[i++];
    while (j <= r) temp[k++] = a[j++];
    for (int t = l; t <= r; ++t) a[t] = temp[t];
    return cnt;
}

long long reversePairs(int* nums, int n) {
    if (nums == NULL || n <= 0) return 0LL;
    int* temp = (int*)malloc(sizeof(int) * n);
    if (!temp) return 0LL;
    // Work on a copy to keep original intact if desired
    int* copy = (int*)malloc(sizeof(int) * n);
    if (!copy) { free(temp); return 0LL; }
    for (int i = 0; i < n; ++i) copy[i] = nums[i];
    long long ans = sortCount(copy, 0, n - 1, temp);
    free(temp);
    free(copy);
    return ans;
}

static void run_case(int* arr, int n) {
    printf("%lld\n", reversePairs(arr, n));
}

int main() {
    int a1[] = {1, 3, 2, 3, 1};
    int a2[] = {2, 4, 3, 5, 1};
    int a3[] = {-5, -5};
    int a4[] = {2147483647, -2147483648, 0};
    int a5[] = {5, 4, 3, 2, 1};

    run_case(a1, 5); // 2
    run_case(a2, 5); // 3
    run_case(a3, 2); // 1
    run_case(a4, 3); // 2
    run_case(a5, 5); // 4
    return 0;
}