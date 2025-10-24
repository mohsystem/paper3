#include <stdio.h>
#include <stdlib.h>

int* two_sum(const int* nums, int n, int target) {
    int* result = (int*)malloc(2 * sizeof(int));
    if (!result) return NULL;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (nums[i] + nums[j] == target) {
                result[0] = i;
                result[1] = j;
                return result;
            }
        }
    }
    // Shouldn't happen per problem constraints; return placeholder
    result[0] = result[1] = -1;
    return result;
}

static void print_result(int* res) {
    if (res && res[0] >= 0 && res[1] >= 0) {
        printf("{%d, %d}\n", res[0], res[1]);
    } else {
        printf("{}\n");
    }
}

int main() {
    int arr1[] = {1, 2, 3};
    int arr2[] = {3, 2, 4};
    int arr3[] = {2, 7, 11, 15};
    int arr4[] = {3, 3};
    int arr5[] = {0, -1, 2, -3, 5};

    int* r1 = two_sum(arr1, 3, 4);
    int* r2 = two_sum(arr2, 3, 6);
    int* r3 = two_sum(arr3, 4, 9);
    int* r4 = two_sum(arr4, 2, 6);
    int* r5 = two_sum(arr5, 5, -1);

    print_result(r1);
    print_result(r2);
    print_result(r3);
    print_result(r4);
    print_result(r5);

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);
    return 0;
}