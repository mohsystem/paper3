#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_SIZE 1000000

// Safely calculate allocation size and allocate buffer.
bool allocate_buffer(size_t size, int start_val, int **out_buf, size_t *out_size) {
    if (out_buf == NULL || out_size == NULL) {
        return false;
    }
    *out_buf = NULL;
    *out_size = 0;

    if (size > MAX_SIZE) {
        return false;
    }
    if (size > 0 && SIZE_MAX / sizeof(int) < size) {
        return false; // overflow check
    }
    int *buf = NULL;
    if (size == 0) {
        // zero-length buffer is allowed; keep as NULL and size 0
        *out_buf = NULL;
        *out_size = 0;
        return true;
    }
    buf = (int *)malloc(size * sizeof(int));
    if (buf == NULL) {
        return false;
    }
    for (size_t i = 0; i < size; ++i) {
        buf[i] = start_val + (int)i;
    }
    *out_buf = buf;
    *out_size = size;
    return true;
}

bool read_at(const int *buf, size_t size, size_t index, int *out_val) {
    if (out_val == NULL) return false;
    if (buf == NULL && size == 0 && index == 0) return false;
    if (index >= size) return false;
    *out_val = buf[index];
    return true;
}

int main(void) {
    // 5 test cases
    int *buf1 = NULL; size_t len1 = 0; int val = 0;
    if (allocate_buffer(10, 100, &buf1, &len1) && read_at(buf1, len1, 0, &val)) {
        printf("Test1: %d\n", val);
    } else {
        printf("Test1: Invalid\n");
    }
    free(buf1);

    int *buf2 = NULL; size_t len2 = 0;
    if (allocate_buffer(10, 200, &buf2, &len2) && read_at(buf2, len2, 9, &val)) {
        printf("Test2: %d\n", val);
    } else {
        printf("Test2: Invalid\n");
    }
    free(buf2);

    int *buf3 = NULL; size_t len3 = 0;
    if (allocate_buffer(10, 300, &buf3, &len3) && read_at(buf3, len3, 10, &val)) {
        printf("Test3: %d\n", val);
    } else {
        printf("Test3: Invalid\n");
    }
    free(buf3);

    int *buf4 = NULL; size_t len4 = 0;
    if (allocate_buffer(0, 400, &buf4, &len4) && read_at(buf4, len4, 0, &val)) {
        printf("Test4: %d\n", val);
    } else {
        printf("Test4: Invalid\n");
    }
    free(buf4);

    int *buf5 = NULL; size_t len5 = 0;
    if (allocate_buffer(5, 500, &buf5, &len5) && read_at(buf5, len5, (size_t)-1, &val)) {
        printf("Test5: %d\n", val);
    } else {
        printf("Test5: Invalid\n");
    }
    free(buf5);

    return 0;
}