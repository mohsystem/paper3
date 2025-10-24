
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Merge sort based solution to count reverse pairs
 * Security: All inputs validated, bounds checked, integer overflow prevented
 */

/* Merge helper that counts cross-boundary reverse pairs
 * Security: Validates all array accesses and checks for integer overflow
 */
static int mergeAndCount(int* nums, int* temp, int left, int mid, int right) {
    if (nums == NULL || temp == NULL || left < 0 || mid < left || right < mid) {
        return 0;
    }
    
    int count = 0;
    int i = left;
    int j = mid + 1;
    
    /* Count reverse pairs where i is in left half and j is in right half
     * Security: Prevent integer overflow in multiplication by casting to long long
     */
    int k = mid + 1;
    for (i = left; i <= mid; i++) {
        /* Use long long to prevent overflow when computing 2 * nums[k] */
        while (k <= right && (long long)nums[i] > 2LL * (long long)nums[k]) {
            k++;
        }
        /* Check for integer overflow in subtraction */
        if (k > mid + 1) {
            count += (k - (mid + 1));
        }
    }
    
    /* Merge the two sorted halves */
    i = left;
    j = mid + 1;
    k = left;
    
    while (i <= mid && j <= right) {
        /* Bounds check on all array accesses */
        if (i > mid || j > right || k > right) {
            break;
        }
        
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    /* Copy remaining elements from left half */
    while (i <= mid && k <= right) {
        temp[k++] = nums[i++];
    }
    
    /* Copy remaining elements from right half */
    while (j <= right && k <= right) {
        temp[k++] = nums[j++];
    }
    
    /* Copy sorted elements back to original array with bounds checking */
    for (i = left; i <= right; i++) {
        if (i < left || i > right) {
            break;
        }
        nums[i] = temp[i];
    }
    
    return count;
}

/* Recursive merge sort that counts reverse pairs
 * Security: Validates inputs and checks for integer overflow
 */
static int mergeSortAndCount(int* nums, int* temp, int left, int right) {
    /* Input validation */
    if (nums == NULL || temp == NULL || left < 0 || right < 0 || left > right) {
        return 0;
    }
    
    int count = 0;
    
    if (left < right) {
        /* Check for integer overflow in mid calculation */
        int mid = left + (right - left) / 2;
        
        /* Recursively count in left and right halves */
        count += mergeSortAndCount(nums, temp, left, mid);
        count += mergeSortAndCount(nums, temp, mid + 1, right);
        
        /* Count cross-boundary pairs and merge */
        count += mergeAndCount(nums, temp, left, mid, right);
    }
    
    return count;
}

/* Main function to count reverse pairs
 * Security: Validates all inputs, checks array bounds and allocation results
 */
int reversePairs(int* nums, int numsSize) {
    /* Input validation: check for NULL pointer and valid size */
    if (nums == NULL || numsSize <= 0) {
        return 0;
    }
    
    /* Validate array size to prevent integer overflow */
    if (numsSize > 50000) {
        return 0;
    }
    
    /* Allocate temporary array with overflow check */
    /* Check: numsSize * sizeof(int) won't overflow since numsSize <= 50000 */
    int* temp = (int*)malloc((size_t)numsSize * sizeof(int));
    if (temp == NULL) {
        /* Memory allocation failed - return 0 */
        return 0;
    }
    
    /* Initialize temp array to zero for safety */
    memset(temp, 0, (size_t)numsSize * sizeof(int));
    
    /* Count reverse pairs using merge sort */
    int result = mergeSortAndCount(nums, temp, 0, numsSize - 1);
    
    /* Clear and free temporary memory */
    memset(temp, 0, (size_t)numsSize * sizeof(int));
    free(temp);
    temp = NULL;
    
    return result;
}

/* Test driver with 5 test cases */
int main(void) {
    /* Test case 1: Example 1 from problem */
    int test1[] = {1, 3, 2, 3, 1};
    int size1 = 5;
    printf("Test 1: ");
    for (int i = 0; i < size1; i++) {
        printf("%d ", test1[i]);
    }
    printf("-> %d\\n", reversePairs(test1, size1));
    
    /* Test case 2: Example 2 from problem */
    int test2[] = {2, 4, 3, 5, 1};
    int size2 = 5;
    printf("Test 2: ");
    for (int i = 0; i < size2; i++) {
        printf("%d ", test2[i]);
    }
    printf("-> %d\\n", reversePairs(test2, size2));
    
    /* Test case 3: Single element */
    int test3[] = {1};
    int size3 = 1;
    printf("Test 3: ");
    for (int i = 0; i < size3; i++) {
        printf("%d ", test3[i]);
    }
    printf("-> %d\\n", reversePairs(test3, size3));
    
    /* Test case 4: Already sorted */
    int test4[] = {1, 2, 3, 4, 5};
    int size4 = 5;
    printf("Test 4: ");
    for (int i = 0; i < size4; i++) {
        printf("%d ", test4[i]);
    }
    printf("-> %d\\n", reversePairs(test4, size4));
    
    /* Test case 5: With negative numbers and edge values */
    int test5[] = {2147483647, -2147483648, 0, 1};
    int size5 = 4;
    printf("Test 5: ");
    for (int i = 0; i < size5; i++) {
        printf("%d ", test5[i]);
    }
    printf("-> %d\\n", reversePairs(test5, size5));
    
    return 0;
}
