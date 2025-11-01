#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int do_memory_ops(const uint8_t* data) {
    const size_t SIZE = 10;
    uint8_t* first = (uint8_t*)malloc(SIZE);
    uint8_t* second = (uint8_t*)malloc(SIZE);

    if (!first || !second) {
        if (first) free(first);
        if (second) free(second);
        return -1;
    }

    memcpy(first, data, SIZE);
    memcpy(second, first, SIZE);

    int sum = 0;
    for (size_t i = 0; i < SIZE; ++i) {
        sum += second[i];
    }

    free(first);  // free first chunk
    free(second); // free second chunk

    return sum;
}

int main(void) {
    uint8_t t1[10] = {0,1,2,3,4,5,6,7,8,9};
    printf("%d\n", do_memory_ops(t1)); // 45

    uint8_t t2[10];
    for (int i = 0; i < 10; ++i) t2[i] = 1;
    printf("%d\n", do_memory_ops(t2)); // 10

    uint8_t t3[10] = {10,11,12,13,14,15,16,17,18,19};
    printf("%d\n", do_memory_ops(t3)); // 145

    uint8_t t4[10];
    for (int i = 0; i < 10; ++i) t4[i] = 255;
    printf("%d\n", do_memory_ops(t4)); // 2550

    uint8_t t5[10] = {5,4,3,2,1,0,250,251,252,253};
    printf("%d\n", do_memory_ops(t5)); // 1021

    return 0;
}