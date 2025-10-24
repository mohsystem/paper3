#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool validate_digits(const int *nums, size_t len) {
    if (nums == NULL) return false;
    if (len != 10) return false;
    for (size_t i = 0; i < len; ++i) {
        if (nums[i] < 0 || nums[i] > 9) return false;
    }
    return true;
}

/*
 * Returns a newly allocated string of the form "(xxx) xxx-xxxx" on success.
 * Caller is responsible for free()'ing the returned pointer.
 * Returns NULL on invalid input or allocation failure.
 */
char* createPhoneNumber(const int *nums, size_t len) {
    if (!validate_digits(nums, len)) {
        return NULL;
    }
    // 14 chars + null terminator
    char *out = (char*)malloc(15);
    if (out == NULL) {
        return NULL;
    }
    // Using snprintf to avoid buffer overflows
    int written = snprintf(out, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
                           nums[0], nums[1], nums[2],
                           nums[3], nums[4], nums[5],
                           nums[6], nums[7], nums[8], nums[9]);
    if (written != 14) {
        free(out);
        return NULL;
    }
    return out;
}

int main(void) {
    int t1[10] = {1,2,3,4,5,6,7,8,9,0};
    int t2[10] = {0,0,0,0,0,0,0,0,0,0};
    int t3[10] = {9,8,7,6,5,4,3,2,1,0};
    int t4[10] = {1,1,1,1,1,1,1,1,1,1};
    int t5[10] = {1,2,3,4,-5,6,7,8,9,0}; // invalid

    const int* tests[] = { t1, t2, t3, t4, t5 };
    size_t lens[] = { 10, 10, 10, 10, 10 };

    for (int i = 0; i < 5; ++i) {
        char* res = createPhoneNumber(tests[i], lens[i]);
        if (res != NULL) {
            printf("Test %d: %s\n", i+1, res);
            free(res);
        } else {
            printf("Test %d: ERROR - invalid input or allocation failure\n", i+1);
        }
    }
    return 0;
}