#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* data;
    int n;
    int calls;
} MountainArray;

void mountain_init(MountainArray* m, int* src, int n) {
    if (!m || !src || n < 0) exit(1);
    m->data = (int*)malloc(sizeof(int) * n);
    if (!m->data) exit(1);
    for (int i = 0; i < n; ++i) m->data[i] = src[i];
    m->n = n;
    m->calls = 0;
}

void mountain_free(MountainArray* m) {
    if (m && m->data) {
        free(m->data);
        m->data = NULL;
        m->n = 0;
        m->calls = 0;
    }
}

int mountain_get(MountainArray* m, int k) {
    if (!m || !m->data || k < 0 || k >= m->n) {
        exit(1);
    }
    m->calls += 1;
    return m->data[k];
}

int mountain_length(MountainArray* m) {
    if (!m) return 0;
    return m->n;
}

int find_peak(MountainArray* m) {
    int lo = 0, hi = mountain_length(m) - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int a = mountain_get(m, mid);
        int b = mountain_get(m, mid + 1);
        if (a < b) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

int bin_asc(MountainArray* m, int lo, int hi, int target) {
    int ans = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = mountain_get(m, mid);
        if (val == target) {
            ans = mid;
            hi = mid - 1;
        } else if (val < target) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return ans;
}

int bin_desc(MountainArray* m, int lo, int hi, int target) {
    int ans = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = mountain_get(m, mid);
        if (val == target) {
            ans = mid;
            hi = mid - 1;
        } else if (val < target) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return ans;
}

int findInMountainArray(int target, MountainArray* mountainArr) {
    int n = mountain_length(mountainArr);
    if (n < 3) return -1;
    int peak = find_peak(mountainArr);
    int left = bin_asc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return bin_desc(mountainArr, peak + 1, n - 1, target);
}

int findInMountainArrayWithRawArray(int* arr, int n, int target) {
    MountainArray m;
    mountain_init(&m, arr, n);
    int res = findInMountainArray(target, &m);
    mountain_free(&m);
    return res;
}

int main() {
    int arr1[] = {1,2,3,4,5,3,1};
    int arr2[] = {0,1,2,4,2,1};
    int arr3[] = {0,5,3,1};
    int arr4[] = {1,2,3,4,5,4,3,2,1};
    int arr5[] = {0,1,0};

    printf("%d\n", findInMountainArrayWithRawArray(arr1, sizeof(arr1)/sizeof(int), 3));
    printf("%d\n", findInMountainArrayWithRawArray(arr2, sizeof(arr2)/sizeof(int), 3));
    printf("%d\n", findInMountainArrayWithRawArray(arr3, sizeof(arr3)/sizeof(int), 1));
    printf("%d\n", findInMountainArrayWithRawArray(arr4, sizeof(arr4)/sizeof(int), 9));
    printf("%d\n", findInMountainArrayWithRawArray(arr5, sizeof(arr5)/sizeof(int), 0));
    return 0;
}