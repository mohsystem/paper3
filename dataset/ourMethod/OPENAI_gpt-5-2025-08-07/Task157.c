#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_CAPACITY 10000000u

typedef struct {
    size_t size;
    unsigned char* data;
    int closed;
} MemoryResource;

static MemoryResource* allocate_buffer(size_t size) {
    if (size == 0 || size > MAX_CAPACITY) {
        return NULL;
    }
    MemoryResource* res = (MemoryResource*)calloc(1, sizeof(MemoryResource));
    if (!res) {
        return NULL;
    }
    res->data = (unsigned char*)calloc(size, sizeof(unsigned char));
    if (!res->data) {
        free(res);
        return NULL;
    }
    res->size = size;
    res->closed = 0;
    return res;
}

static int write_buffer(MemoryResource* res, size_t offset, const unsigned char* input, size_t input_len) {
    if (!res || !input || res->closed) return -1;
    if (offset >= res->size) return -1;
    size_t writable = input_len;
    size_t remaining = res->size - offset;
    if (writable > remaining) writable = remaining;
    if (writable == 0) return -1;
    memcpy(res->data + offset, input, writable);
    if (writable > INT32_MAX) return -1; // clamp to int return type
    return (int)writable;
}

static size_t read_buffer(MemoryResource* res, size_t offset, size_t length, unsigned char* out, size_t out_size) {
    if (!res || !out || res->closed) return 0;
    if (offset >= res->size) return 0;
    size_t readable = length;
    size_t remaining = res->size - offset;
    if (readable > remaining) readable = remaining;
    if (readable > out_size) readable = out_size;
    if (readable == 0) return 0;
    memcpy(out, res->data + offset, readable);
    return readable;
}

static int free_buffer(MemoryResource** resPtr) {
    if (!resPtr || !*resPtr) return -1;
    MemoryResource* res = *resPtr;
    if (!res->closed && res->data) {
        // Zeroize before free
        memset(res->data, 0, res->size);
    }
    free(res->data);
    res->data = NULL;
    res->size = 0;
    res->closed = 1;
    free(res);
    *resPtr = NULL;
    return 0;
}

static int close_buffer(MemoryResource* res) {
    if (!res) return -1;
    if (res->closed) return 0;
    if (res->data && res->size > 0) {
        memset(res->data, 0, res->size);
        free(res->data);
        res->data = NULL;
    }
    res->size = 0;
    res->closed = 1;
    return 0;
}

static int is_closed(MemoryResource* res) {
    if (!res) return 1;
    return res->closed ? 1 : 0;
}

int main(void) {
    // Test 1: Basic allocation, write, read
    MemoryResource* mr1 = allocate_buffer(16);
    if (!mr1) {
        printf("Allocation failed\n");
        return 1;
    }
    int w1 = write_buffer(mr1, 0, (const unsigned char*)"Hello", 5);
    unsigned char buf1[6];
    size_t r1 = read_buffer(mr1, 0, 5, buf1, sizeof(buf1) - 1);
    buf1[r1] = '\0';
    printf("T1: written=%d read=\"%s\" cap=%zu\n", w1, buf1, mr1->size);
    close_buffer(mr1); // Free internals but keep struct to demonstrate closed state (not used further)
    // Now fully free
    free_buffer(&mr1);

    // Test 2: Partial write near end
    MemoryResource* mr2 = allocate_buffer(16);
    int w2 = write_buffer(mr2, 13, (const unsigned char*)"World!", 6); // expect 3
    unsigned char buf2[17];
    size_t r2 = read_buffer(mr2, 0, 16, buf2, sizeof(buf2) - 1);
    buf2[r2] = '\0';
    printf("T2: written=%d read=\"%s\"\n", w2, buf2);
    free_buffer(&mr2);

    // Test 3: Write larger than capacity, ensure clamp
    MemoryResource* mr3 = allocate_buffer(8);
    int w3 = write_buffer(mr3, 0, (const unsigned char*)"123456789", 9); // expect 8
    unsigned char buf3[9];
    size_t r3 = read_buffer(mr3, 0, 8, buf3, sizeof(buf3) - 1);
    buf3[r3] = '\0';
    printf("T3: written=%d read=\"%s\"\n", w3, buf3);
    free_buffer(&mr3);

    // Test 4: Use after close should fail
    MemoryResource* mr4 = allocate_buffer(10);
    close_buffer(mr4);
    int w4 = write_buffer(mr4, 0, (const unsigned char*)"X", 1); // expect -1
    unsigned char buf4[2];
    size_t r4 = read_buffer(mr4, 0, 1, buf4, sizeof(buf4));
    printf("T4: written=%d readLen=%zu isClosed=%d\n", w4, r4, is_closed(mr4));
    free_buffer(&mr4);

    // Test 5: Invalid offset (too large)
    MemoryResource* mr5 = allocate_buffer(4);
    int w5 = write_buffer(mr5, 100, (const unsigned char*)"AB", 2); // expect -1
    printf("T5: written=%d cap=%zu\n", w5, mr5->size);
    free_buffer(&mr5);

    return 0;
}