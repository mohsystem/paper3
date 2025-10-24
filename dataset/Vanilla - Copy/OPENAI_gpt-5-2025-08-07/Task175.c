#include <stdio.h>
#include <stdlib.h>

typedef struct MountainArray MountainArray;
struct MountainArray {
    int (*get)(MountainArray*, int);
    int (*length)(MountainArray*);
    void* obj;
};

typedef struct {
    int* arr;
    int n;
    int calls;
} SimpleMA;

int sma_get(MountainArray* m, int index) {
    SimpleMA* s = (SimpleMA*)m->obj;
    s->calls++;
    return s->arr[index];
}

int sma_length(MountainArray* m) {
    SimpleMA* s = (SimpleMA*)m->obj;
    return s->n;
}

void makeSimpleMA(MountainArray* m, int* arr, int n) {
    SimpleMA* s = (SimpleMA*)malloc(sizeof(SimpleMA));
    s->arr = arr;
    s->n = n;
    s->calls = 0;
    m->get = sma_get;
    m->length = sma_length;
    m->obj = s;
}

void freeSimpleMA(MountainArray* m) {
    if (m && m->obj) {
        free(m->obj);
        m->obj = NULL;
    }
}

int findInMountainArray(int target, MountainArray* mountainArr);

static int findPeak(MountainArray* m, int n) {
    int l = 0, r = n - 1;
    while (l < r) {
        int mid = l + (r - l) / 2;
        int a = m->get(m, mid);
        int b = m->get(m, mid + 1);
        if (a < b) l = mid + 1;
        else r = mid;
    }
    return l;
}

static int binarySearchAsc(MountainArray* m, int l, int r, int target) {
    while (l <= r) {
        int mid = l + (r - l) / 2;
        int val = m->get(m, mid);
        if (val == target) return mid;
        if (val < target) l = mid + 1;
        else r = mid - 1;
    }
    return -1;
}

static int binarySearchDesc(MountainArray* m, int l, int r, int target) {
    while (l <= r) {
        int mid = l + (r - l) / 2;
        int val = m->get(m, mid);
        if (val == target) return mid;
        if (val < target) r = mid - 1;
        else l = mid + 1;
    }
    return -1;
}

int findInMountainArray(int target, MountainArray* mountainArr) {
    int n = mountainArr->length(mountainArr);
    int peak = findPeak(mountainArr, n);
    int left = binarySearchAsc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binarySearchDesc(mountainArr, peak + 1, n - 1, target);
}

int main() {
    int arr1[] = {1,2,3,4,5,3,1};
    int arr2[] = {0,1,2,4,2,1};
    int arr3[] = {0,5,3,1};
    int arr4[] = {0,2,4,5,3,1};
    int arr5[] = {0,2,4,5,3,1};
    int targets[] = {3, 3, 1, 0, 1};
    int* arrays[] = {arr1, arr2, arr3, arr4, arr5};
    int sizes[] = {7, 6, 4, 6, 6};

    for (int i = 0; i < 5; ++i) {
        MountainArray m;
        makeSimpleMA(&m, arrays[i], sizes[i]);
        int res = findInMountainArray(targets[i], &m);
        printf("%d\n", res);
        freeSimpleMA(&m);
    }
    return 0;
}