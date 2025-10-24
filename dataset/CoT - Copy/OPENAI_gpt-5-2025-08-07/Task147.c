#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
Step 1 & 2: Understand and secure
- Validate inputs to prevent invalid memory access.
- Do not modify the input array; sort a copy instead.
*/

// Comparator for descending order, avoiding overflow by not subtracting directly
static int cmp_desc(const void* a, const void* b) {
    int va = *(const int*)a;
    int vb = *(const int*)b;
    if (va > vb) return -1;
    if (va < vb) return 1;
    return 0;
}

/*
Step 3: Secure coding generation
Function: Finds the kth largest element in the array.
Parameters:
- nums: pointer to input array
- n: number of elements in nums
- k: which largest to find (1-based)
- ok: output flag set to 1 on success, 0 on failure (may be NULL)
Returns: the kth largest element if ok==1, otherwise 0 (undefined if failure)
*/
int findKthLargest(const int* nums, size_t n, size_t k, int* ok) {
    if (ok) *ok = 0;
    if (nums == NULL || n == 0 || k == 0 || k > n) {
        return 0;
    }

    int* copy = (int*)malloc(n * sizeof(int));
    if (copy == NULL) {
        return 0;
    }

    // Copy input safely
    memcpy(copy, nums, n * sizeof(int));

    // Sort in descending order
    qsort(copy, n, sizeof(int), cmp_desc);

    int result = copy[k - 1];
    free(copy);

    if (ok) *ok = 1;
    return result;
}

// Step 5: Main with 5 test cases
int main(void) {
    int ok = 0;

    int arr1[] = {3, 2, 1, 5, 6, 4};
    int res1 = findKthLargest(arr1, sizeof(arr1)/sizeof(arr1[0]), 2, &ok);
    if (ok) printf("Test 1 -> kth largest: %d\n", res1);

    int arr2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int res2 = findKthLargest(arr2, sizeof(arr2)/sizeof(arr2[0]), 4, &ok);
    if (ok) printf("Test 2 -> kth largest: %d\n", res2);

    int arr3[] = {-1, -3, -2, -4, -5};
    int res3 = findKthLargest(arr3, sizeof(arr3)/sizeof(arr3[0]), 2, &ok);
    if (ok) printf("Test 3 -> kth largest: %d\n", res3);

    int arr4[] = {7};
    int res4 = findKthLargest(arr4, sizeof(arr4)/sizeof(arr4[0]), 1, &ok);
    if (ok) printf("Test 4 -> kth largest: %d\n", res4);

    int arr5[] = {9, 9, 8, 7, 6, 5};
    int res5 = findKthLargest(arr5, sizeof(arr5)/sizeof(arr5[0]), 3, &ok);
    if (ok) printf("Test 5 -> kth largest: %d\n", res5);

    return 0;
}