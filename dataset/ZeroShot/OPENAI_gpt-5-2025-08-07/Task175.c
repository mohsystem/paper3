#include <stdio.h>
#include <limits.h>

typedef struct {
    const int* arr;
    int n;
    int getCount;
    int maxGets;
} MountainArray;

static int ma_length(const MountainArray* m) {
    if (m == NULL) return 0;
    return m->n;
}

static int ma_get(MountainArray* m, int k) {
    if (m == NULL) return INT_MIN;
    if (k < 0 || k >= m->n) {
        return INT_MIN; // indicate error; caller avoids invalid indices
    }
    if (m->maxGets > 0 && m->getCount + 1 > m->maxGets) {
        return INT_MIN; // exceeded; treat as error
    }
    m->getCount += 1;
    return m->arr[k];
}

static int findPeakC(MountainArray* m) {
    int lo = 0, hi = ma_length(m) - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int a = ma_get(m, mid);
        int b = ma_get(m, mid + 1);
        if (a == INT_MIN || b == INT_MIN) return -1; // error guard
        if (a < b) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

static int binSearchAscC(MountainArray* m, int lo, int hi, int target) {
    int res = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = ma_get(m, mid);
        if (val == INT_MIN) return res;
        if (val == target) {
            res = mid;
            hi = mid - 1;
        } else if (val < target) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return res;
}

static int binSearchDescC(MountainArray* m, int lo, int hi, int target) {
    int res = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = ma_get(m, mid);
        if (val == INT_MIN) return res;
        if (val == target) {
            res = mid;
            hi = mid - 1;
        } else if (val > target) { // descending
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return res;
}

int findInMountainArrayC(int target, MountainArray* mountainArr) {
    if (mountainArr == NULL) return -1;
    int n = ma_length(mountainArr);
    if (n < 3) return -1;
    int peak = findPeakC(mountainArr);
    if (peak < 0) return -1;
    int left = binSearchAscC(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binSearchDescC(mountainArr, peak + 1, n - 1, target);
}

int main(void) {
    int arr1[] = {1,2,3,4,5,3,1};
    int arr2[] = {0,1,2,4,2,1};
    int arr3[] = {0,5,3,1};
    int arr4[] = {1,5,2};
    int arr5[] = {1,2,3,4,5,6,7,6,5,4,3,2,1};

    MountainArray m1 = {arr1, (int)(sizeof(arr1)/sizeof(arr1[0])), 0, 1000};
    MountainArray m2 = {arr2, (int)(sizeof(arr2)/sizeof(arr2[0])), 0, 1000};
    MountainArray m3 = {arr3, (int)(sizeof(arr3)/sizeof(arr3[0])), 0, 1000};
    MountainArray m4 = {arr4, (int)(sizeof(arr4)/sizeof(arr4[0])), 0, 1000};
    MountainArray m5 = {arr5, (int)(sizeof(arr5)/sizeof(arr5[0])), 0, 1000};

    printf("%d\n", findInMountainArrayC(3, &m1));
    printf("%d\n", findInMountainArrayC(3, &m2));
    printf("%d\n", findInMountainArrayC(1, &m3));
    printf("%d\n", findInMountainArrayC(2, &m4));
    printf("%d\n", findInMountainArrayC(7, &m5));

    return 0;
}