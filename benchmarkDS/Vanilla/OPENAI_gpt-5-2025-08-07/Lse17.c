#include <stdio.h>
#include <stdlib.h>

int getValueAtIndex(const int* arr, int len, int index) {
    if (arr == NULL) {
        fprintf(stderr, "Array is NULL\n");
        exit(1);
    }
    if (index < 0 || index >= len) {
        fprintf(stderr, "Invalid index\n");
        exit(1);
    }
    return arr[index];
}

int main(void) {
    int arr[] = {10, 25, 37, 48, 59};
    int len = (int)(sizeof(arr) / sizeof(arr[0]));
    int testIndices[5] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; ++i) {
        int value = getValueAtIndex(arr, len, testIndices[i]);
        printf("%d\n", value);
    }
    return 0;
}