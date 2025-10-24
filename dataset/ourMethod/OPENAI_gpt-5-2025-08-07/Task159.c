#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1048576

struct ReadResult {
    int success;          /* 1 if success, 0 otherwise */
    unsigned int value;   /* holds the read value if success */
};

unsigned char* allocate_buffer(size_t size) {
    if (size > MAX_BUFFER_SIZE) {
        return NULL;
    }
    unsigned char* buf = (unsigned char*)malloc(size);
    if (buf == NULL) {
        return NULL;
    }
    return buf;
}

void fill_sequential(unsigned char* buffer, size_t size) {
    if (buffer == NULL) {
        return;
    }
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (unsigned char)(i & 0xFF);
    }
}

struct ReadResult safe_read(const unsigned char* buffer, size_t size, int index) {
    struct ReadResult res;
    if (buffer == NULL) {
        res.success = 0;
        res.value = 0;
        return res;
    }
    if (index < 0) {
        res.success = 0;
        res.value = 0;
        return res;
    }
    size_t uidx = (size_t)index;
    if (uidx >= size) {
        res.success = 0;
        res.value = 0;
        return res;
    }
    res.success = 1;
    res.value = (unsigned int)buffer[uidx];
    return res;
}

int main(void) {
    size_t size = 10;
    unsigned char* buffer = allocate_buffer(size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer\n");
        return 1;
    }
    fill_sequential(buffer, size);

    int testIndices[5] = { -1, 0, 5, 9, 10 };
    for (int i = 0; i < 5; i++) {
        int idx = testIndices[i];
        struct ReadResult r = safe_read(buffer, size, idx);
        if (r.success) {
            printf("Index %d -> value: %u\n", idx, r.value);
        } else {
            printf("Index %d -> error: out of bounds\n", idx);
        }
    }

    printf("Buffer length: %lu\n", (unsigned long)size);
    printf("Buffer content: [");
    for (size_t i = 0; i < size; i++) {
        printf("%u", (unsigned int)buffer[i]);
        if (i + 1 < size) {
            printf(", ");
        }
    }
    printf("]\n");

    free(buffer);
    return 0;
}