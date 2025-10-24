#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Performs a binary search on a sorted integer array.
 *
 * @param arr    The sorted array to search in. Must not be NULL.
 * @param size   The number of elements in the array.
 * @param target The value to search for.
 * @return The index of the target, or -1 if not found.
 */
int binarySearch(const int* arr, int size, int target) {
    if (arr == NULL || size <= 0) {
        return -1;
    }
    
    int low = 0;
    int high = size - 1;

    while (low <= high) {
        // Use `low + (high - low) / 2` to prevent potential overflow
        int mid = low + (high - low) / 2;

        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1; // Target not found
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]");
}

int main(void) {
    const int sortedArray[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    const int arraySize = sizeof(sortedArray) / sizeof(sortedArray[0]);
    const int testTargets[] = {23, 91, 2, 15, 100};
    const int numTests = sizeof(testTargets) / sizeof(testTargets[0]);

    // Creating a file in the current working directory for portability.
    // Secure temp file creation is platform-specific (e.g., mkstemp on POSIX).
    const char* filename = "task87_c_times.txt";
    FILE* fp = fopen(filename, "w");

    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf("Execution times will be written to: %s\n", filename);

    for (int i = 0; i < numTests; ++i) {
        int target = testTargets[i];
        
        // Using clock() for portability. For higher precision, consider
        // clock_gettime on POSIX or QueryPerformanceCounter on Windows.
        clock_t start = clock();
        int result = binarySearch(sortedArray, arraySize, target);
        clock_t end = clock();

        double duration_ms = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;

        printf("Array: ");
        printArray(sortedArray, arraySize);
        printf("\n");
        printf("Searching for: %d\n", target);
        if (result != -1) {
            printf("Found at index: %d\n", result);
        } else {
            printf("Not found.\n");
        }
        printf("Execution time: %f milliseconds.\n\n", duration_ms);

        if (fprintf(fp, "Target: %d, Time (ms): %f\n", target, duration_ms) < 0) {
            fprintf(stderr, "Error writing to file for target %d\n", target);
        }
    }

    if (fclose(fp) != 0) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}