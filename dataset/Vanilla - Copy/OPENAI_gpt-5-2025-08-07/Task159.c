#include <stdio.h>
#include <stdlib.h>

// Dynamically allocates an int buffer, fills it, and returns the value at the given index.
// Returns -1 for invalid size, allocation failure, or out-of-bounds index.
int readFromBuffer(size_t size, long long index, int fillBase) {
    if (size == 0) return -1;
    int* buffer = (int*)malloc(size * sizeof(int));
    if (!buffer) return -1;

    for (size_t i = 0; i < size; ++i) {
        buffer[i] = fillBase + (int)i;
    }

    int result = -1;
    if (index >= 0 && (size_t)index < size) {
        result = buffer[index];
    }

    free(buffer);
    return result;
}

int main(void) {
    size_t size = 5;
    int fillBase = 100;
    long long testIndices[5] = {0, 4, 5, -1, 2};

    for (int i = 0; i < 5; ++i) {
        long long idx = testIndices[i];
        int result = readFromBuffer(size, idx, fillBase);
        printf("Index %lld -> %d\n", idx, result);
    }
    return 0;
}