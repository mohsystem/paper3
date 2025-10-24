#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int validate_digits(const int *digits, size_t len) {
    if (digits == NULL) return 0;
    if (len != 10) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (digits[i] < 0 || digits[i] > 9) return 0;
    }
    return 1;
}

char* createPhoneNumber(const int *digits, size_t len) {
    if (!validate_digits(digits, len)) {
        return NULL;
    }
    // "(xxx) xxx-xxxx" = 14 chars + 1 null terminator
    size_t bufsize = 15;
    char *out = (char*)malloc(bufsize);
    if (out == NULL) {
        return NULL;
    }
    int written = snprintf(out, bufsize, "(%d%d%d) %d%d%d-%d%d%d%d",
                           digits[0], digits[1], digits[2],
                           digits[3], digits[4], digits[5],
                           digits[6], digits[7], digits[8], digits[9]);
    if (written < 0 || (size_t)written >= bufsize) {
        free(out);
        return NULL;
    }
    return out;
}

static void run_test(const int *digits, size_t len, int test_num) {
    char *result = createPhoneNumber(digits, len);
    if (result == NULL) {
        printf("Test %d: Invalid input\n", test_num);
    } else {
        printf("Test %d: %s\n", test_num, result);
        free(result);
    }
}

int main(void) {
    int t1[10] = {1,2,3,4,5,6,7,8,9,0};
    int t2[10] = {0,0,0,0,0,0,0,0,0,0};
    int t3[10] = {9,8,7,6,5,4,3,2,1,0};
    int t4[10] = {5,5,5,1,2,1,2,3,4,5};
    int t5[10] = {2,1,2,5,5,5,0,1,0,0};

    run_test(t1, 10, 1);
    run_test(t2, 10, 2);
    run_test(t3, 10, 3);
    run_test(t4, 10, 4);
    run_test(t5, 10, 5);

    return 0;
}