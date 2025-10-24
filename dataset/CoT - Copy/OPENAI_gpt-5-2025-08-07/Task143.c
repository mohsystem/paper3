#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

/*
 Chain-of-Through process:
 1. Problem understanding:
    - Sort an array of integers ascending. Function accepts pointer and length, returns newly allocated sorted array.
 2. Security requirements:
    - Avoid modifying input; check for NULL and size issues; prevent overflow in allocations; safe comparator to avoid overflow.
 3. Secure coding generation:
    - Allocate a copy with overflow checks; use qsort with safe comparator; caller must free result.
 4. Code review:
    - No buffer overreads/writes; comparator avoids subtraction overflow; handles n == 0 or NULL gracefully.
 5. Secure code output:
    - Final function below.
*/

static int safe_int_cmp(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

// Returns a newly allocated sorted copy of arr with length n.
// On error or if n == 0, returns NULL. Caller is responsible for free().
int* sort_array(const int* arr, size_t n) {
    if (arr == NULL || n == 0) {
        return NULL;
    }
    if (n > SIZE_MAX / sizeof(int)) { // prevent overflow
        return NULL;
    }
    int* copy = (int*)malloc(n * sizeof(int));
    if (copy == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; ++i) {
        copy[i] = arr[i];
    }
    qsort(copy, n, sizeof(int), safe_int_cmp);
    return copy;
}

static void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("%d", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    // 5 test cases
    int t1[] = {};                        // empty
    size_t n1 = 0;

    int t2[] = {1, 2, 3, 4, 5};           // already sorted
    size_t n2 = sizeof(t2)/sizeof(t2[0]);

    int t3[] = {5, 4, 3, 2, 1};           // reverse
    size_t n3 = sizeof(t3)/sizeof(t3[0]);

    int t4[] = {3, 1, 2, 1, 3, 2, 2, 1};  // duplicates
    size_t n4 = sizeof(t4)/sizeof(t4[0]);

    int t5[] = {-10, 0, 500000, -200, 42, 42}; // negatives and large values
    size_t n5 = sizeof(t5)/sizeof(t5[0]);

    int* s1 = sort_array(t1, n1);
    int* s2 = sort_array(t2, n2);
    int* s3 = sort_array(t3, n3);
    int* s4 = sort_array(t4, n4);
    int* s5 = sort_array(t5, n5);

    if (s1) { print_array(s1, n1); free(s1); } else { print_array(t1, n1); }
    if (s2) { print_array(s2, n2); free(s2); }
    if (s3) { print_array(s3, n3); free(s3); }
    if (s4) { print_array(s4, n4); free(s4); }
    if (s5) { print_array(s5, n5); free(s5); }

    return 0;
}