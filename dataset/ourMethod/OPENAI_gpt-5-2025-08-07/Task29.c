#include <stdio.h>
#include <stddef.h>

typedef struct {
    int index1;
    int index2;
    int found; /* 1 if found, 0 otherwise */
} Pair;

Pair two_sum(const int* numbers, size_t length, int target) {
    Pair result = { -1, -1, 0 };
    if (numbers == NULL || length < 2) {
        return result;
    }
    for (size_t i = 0; i + 1 < length; ++i) {
        for (size_t j = i + 1; j < length; ++j) {
            long long sum = (long long)numbers[i] + (long long)numbers[j];
            if (sum == (long long)target) {
                result.index1 = (int)i;
                result.index2 = (int)j;
                result.found = 1;
                return result;
            }
        }
    }
    return result;
}

static void print_result(Pair p) {
    if (p.found) {
        printf("(%d, %d)", p.index1, p.index2);
    } else {
        printf("(not found)");
    }
}

int main(void) {
    /* Test 1 */
    {
        int a[] = {1, 2, 3};
        int target = 4;
        Pair res = two_sum(a, sizeof(a)/sizeof(a[0]), target);
        printf("Test 1: ");
        print_result(res);
        printf("\n");
    }
    /* Test 2 */
    {
        int a[] = {3, 2, 4};
        int target = 6;
        Pair res = two_sum(a, sizeof(a)/sizeof(a[0]), target);
        printf("Test 2: ");
        print_result(res);
        printf("\n");
    }
    /* Test 3 */
    {
        int a[] = {2, 7, 11, 15};
        int target = 9;
        Pair res = two_sum(a, sizeof(a)/sizeof(a[0]), target);
        printf("Test 3: ");
        print_result(res);
        printf("\n");
    }
    /* Test 4 */
    {
        int a[] = {0, 4, 3, 0};
        int target = 0;
        Pair res = two_sum(a, sizeof(a)/sizeof(a[0]), target);
        printf("Test 4: ");
        print_result(res);
        printf("\n");
    }
    /* Test 5 */
    {
        int a[] = {-3, 4, 3, 90};
        int target = 0;
        Pair res = two_sum(a, sizeof(a)/sizeof(a[0]), target);
        printf("Test 5: ");
        print_result(res);
        printf("\n");
    }

    return 0;
}