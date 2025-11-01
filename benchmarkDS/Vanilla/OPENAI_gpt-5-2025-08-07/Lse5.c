#include <stdio.h>
#include <stdlib.h>

int getElement(const int arr[], int index) {
    return arr[index];
}

int main(int argc, char* argv[]) {
    int arr[] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
    int n = (int)(sizeof(arr) / sizeof(arr[0]));

    if (argc > 1) {
        char* endptr = NULL;
        long idxl = strtol(argv[1], &endptr, 10);
        if (endptr == argv[1] || *endptr != '\0') {
            printf("Invalid integer argument\n");
            return 0;
        }
        if (idxl >= 0 && idxl < n) {
            int val = getElement(arr, (int)idxl);
            printf("%d\n", val);
        } else {
            printf("Index out of bounds\n");
        }
        return 0;
    }

    int tests[] = {0, 1, 4, 7, 9};
    int t = (int)(sizeof(tests) / sizeof(tests[0]));
    for (int i = 0; i < t; ++i) {
        int idx = tests[i];
        if (idx >= 0 && idx < n) {
            int val = getElement(arr, idx);
            printf("%d\n", val);
        }
    }

    return 0;
}