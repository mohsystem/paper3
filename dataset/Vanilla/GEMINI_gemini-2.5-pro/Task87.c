#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Performs binary search on a sorted array.
 *
 * @param arr The sorted array to search in.
 * @param size The number of elements in the array.
 * @param target The element to search for.
 * @return The index of the target element, or -1 if not found.
 */
int binarySearch(const int* arr, int size, int target) {
    int left = 0;
    int right = size - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid; // Element found
        }

        if (arr[mid] < target) {
            left = mid + 1; // Search in the right half
        } else {
            right = mid - 1; // Search in the left half
        }
    }
    return -1; // Element not found
}

/**
 * Writes the execution time to a temporary log file.
 *
 * @param duration The execution time in nanoseconds.
 * @param target The target value that was searched for.
 */
void writeExecutionTime(long long duration, int target) {
    // Using a fixed filename for simplicity, acts as a temporary log.
    FILE *fp = fopen("c_execution_log.tmp", "a");
    if (fp != NULL) {
        fprintf(fp, "Target: %d, Execution Time: %lld ns\n", target, duration);
        fclose(fp);
    } else {
        perror("Error writing to temporary file");
    }
}

void printArray(const int* arr, int size) {
    printf("Array: [");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // The array must be sorted for binary search to work.
    int sortedArray[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    int size = sizeof(sortedArray) / sizeof(sortedArray[0]);
    int testCases[] = {23, 91, 2, 15, 100};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Starting C Binary Search Test Cases...\n");
    printArray(sortedArray, size);
    
    for (int i = 0; i < numTestCases; ++i) {
        int target = testCases[i];
        printf("----------------------------------------\n");

        // High-resolution timer for POSIX systems (Linux, macOS)
        #if defined(__linux__) || defined(__APPLE__)
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);
            int result = binarySearch(sortedArray, size, target);
            clock_gettime(CLOCK_MONOTONIC, &end);
            long long duration = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
        #else
            // Fallback for other systems (like Windows) using standard clock()
            // Note: Resolution may be low, often resulting in 0 ns.
            clock_t start, end;
            start = clock();
            int result = binarySearch(sortedArray, size, target);
            end = clock();
            long long duration = (long long)(((double)(end - start) / CLOCKS_PER_SEC) * 1e9);
        #endif

        printf("Searching for: %d\n", target);
        if (result != -1) {
            printf("Element found at index: %d\n", result);
        } else {
            printf("Element not found.\n");
        }
        printf("Execution time: %lld nanoseconds.\n", duration);
        
        // Write the execution time to a temporary file
        writeExecutionTime(duration, target);
    }
    printf("----------------------------------------\n");
    printf("All execution times have been logged to c_execution_log.tmp\n");

    return 0;
}