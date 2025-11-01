#include <stdio.h>

int getValueAtIndex(int index) {
    int arr[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    return arr[index];
}

int main() {
    int tests[] = {0, 1, 5, 8, 9};
    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; i++) {
        int idx = tests[i];
        int result = getValueAtIndex(idx);
        printf("Index %d -> %d\n", idx, result);
    }
    return 0;
}