#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static void sift_up(int* heap, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap[parent] <= heap[idx]) break;
        int tmp = heap[parent];
        heap[parent] = heap[idx];
        heap[idx] = tmp;
        idx = parent;
    }
}

static void sift_down(int* heap, size_t size, size_t idx) {
    while (1) {
        size_t left = idx * 2 + 1;
        size_t right = idx * 2 + 2;
        size_t smallest = idx;
        if (left < size && heap[left] < heap[smallest]) smallest = left;
        if (right < size && heap[right] < heap[smallest]) smallest = right;
        if (smallest == idx) break;
        int tmp = heap[idx];
        heap[idx] = heap[smallest];
        heap[smallest] = tmp;
        idx = smallest;
    }
}

int kth_largest(const int* nums, size_t n, size_t k) {
    if (nums == NULL || n == 0 || k == 0 || k > n) {
        errno = EINVAL;
        return INT_MIN;
    }
    if (k > (SIZE_MAX / sizeof(int))) {
        errno = EOVERFLOW;
        return INT_MIN;
    }
    int* heap = (int*)malloc(k * sizeof(int));
    if (!heap) {
        errno = ENOMEM;
        return INT_MIN;
    }
    size_t heap_size = 0;
    for (size_t i = 0; i < n; ++i) {
        int v = nums[i];
        if (heap_size < k) {
            heap[heap_size] = v;
            sift_up(heap, heap_size);
            heap_size++;
        } else {
            if (v > heap[0]) {
                heap[0] = v;
                sift_down(heap, heap_size, 0);
            }
        }
    }
    int result = heap[0];
    free(heap);
    return result;
}

int main(void) {
    int arr1[] = {3, 2, 1, 5, 6, 4};
    int arr2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int arr3[] = {-1, -1};
    int arr4[] = {7};
    int arr5[] = {3, 2, 1, 5, 6, 4};

    errno = 0;
    int res1 = kth_largest(arr1, sizeof(arr1)/sizeof(arr1[0]), 2);
    if (errno) { perror("Test 1 error"); } else { printf("Test 1 result: %d\n", res1); }

    errno = 0;
    int res2 = kth_largest(arr2, sizeof(arr2)/sizeof(arr2[0]), 4);
    if (errno) { perror("Test 2 error"); } else { printf("Test 2 result: %d\n", res2); }

    errno = 0;
    int res3 = kth_largest(arr3, sizeof(arr3)/sizeof(arr3[0]), 2);
    if (errno) { perror("Test 3 error"); } else { printf("Test 3 result: %d\n", res3); }

    errno = 0;
    int res4 = kth_largest(arr4, sizeof(arr4)/sizeof(arr4[0]), 1);
    if (errno) { perror("Test 4 error"); } else { printf("Test 4 result: %d\n", res4); }

    errno = 0;
    int res5 = kth_largest(arr5, sizeof(arr5)/sizeof(arr5[0]), 10); // invalid
    if (errno) { perror("Test 5 error"); } else { printf("Test 5 result: %d\n", res5); }

    return 0;
}