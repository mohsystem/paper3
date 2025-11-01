#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int data[10];
} Arr10;

Arr10 copyAndFree(const Arr10* src) {
    int* first = (int*)malloc(10 * sizeof(int));
    for (int i = 0; i < 10; ++i) first[i] = src->data[i];

    int* second = (int*)malloc(10 * sizeof(int));
    for (int i = 0; i < 10; ++i) second[i] = first[i];

    Arr10 result;
    for (int i = 0; i < 10; ++i) result.data[i] = second[i];

    free(first);
    free(second);

    return result;
}

static void print_arr10(const Arr10* a) {
    for (int i = 0; i < 10; ++i) {
        printf("%d%s", a->data[i], (i + 1 < 10) ? " " : "\n");
    }
}

int main() {
    Arr10 tests[5] = {
        {.data = {0,1,2,3,4,5,6,7,8,9}},
        {.data = {10,11,12,13,14,15,16,17,18,19}},
        {.data = {5,5,5,5,5,5,5,5,5,5}},
        {.data = {-5,-4,-3,-2,-1,0,1,2,3,4}},
        {.data = {42,0,-7,13,13,-1,2,2,999,-999}}
    };

    for (int i = 0; i < 5; ++i) {
        Arr10 out = copyAndFree(&tests[i]);
        printf("Test %d: ", i + 1);
        print_arr10(&out);
    }

    return 0;
}