#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Performs a binary search on a sorted array of integers.
 * This implementation assumes the input array is sorted.
 * 
 * @param arr The sorted array to search in.
 * @param size The number of elements in the array.
 * @param target The integer to search for.
 * @return The index of the target, or -1 if not found.
 */
int binarySearch(int arr[], int size, int target) {
    if (arr == NULL || size <= 0) {
        return -1;
    }

    int low = 0;
    int high = size - 1;

    while (low <= high) {
        // Mitigates integer overflow
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

/**
 * @brief Executes binary search, measures its time, and logs it to a temporary file.
 * Uses clock() for timing, which measures processor time and may have low resolution.
 * Uses tmpfile() which creates a secure temporary file that is automatically
 * deleted when the program terminates or the file is closed.
 * 
 * @param arr The sorted array to search in.
 * @param size The number of elements in the array.
 * @param target The integer to search for.
 * @return The index of the target, or -1 if not found.
 */
int performSearchAndLogTime(int arr[], int size, int target) {
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int result = binarySearch(arr, size, target);
    end = clock();
    
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // tmpfile() creates a unique temporary file that is automatically removed on close.
    // This is the most secure method in the C standard library.
    FILE *temp_f = tmpfile();

    if (temp_f == NULL) {
        perror("Error creating temporary file");
        return result;
    }

    fprintf(temp_f, "Binary search for target %d took %f seconds. Result index: %d\n", target, cpu_time_used, result);
    // You could rewind and read the file here if needed (fseek, fgets)
    // The file will be deleted upon closing.
    fclose(temp_f); 
    
    printf("Execution time was written to a secure temporary file (which is now deleted).\n");
    
    return result;
}


int main() {
    // The array must be sorted for binary search to work correctly.
    int sortedArray[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    int n = sizeof(sortedArray) / sizeof(sortedArray[0]);

    printf("Running test cases...\n");

    // Test Case 1: Element in the middle
    int target1 = 23;
    printf("Searching for %d... Found at index: %d\n\n", target1, performSearchAndLogTime(sortedArray, n, target1));

    // Test Case 2: Element at the beginning
    int target2 = 2;
    printf("Searching for %d... Found at index: %d\n\n", target2, performSearchAndLogTime(sortedArray, n, target2));

    // Test Case 3: Element at the end
    int target3 = 91;
    printf("Searching for %d... Found at index: %d\n\n", target3, performSearchAndLogTime(sortedArray, n, target3));

    // Test Case 4: Element not found (smaller than all)
    int target4 = 1;
    printf("Searching for %d... Found at index: %d\n\n", target4, performSearchAndLogTime(sortedArray, n, target4));

    // Test Case 5: Element not found (larger than all)
    int target5 = 100;
    printf("Searching for %d... Found at index: %d\n\n", target5, performSearchAndLogTime(sortedArray, n, target5));

    return 0;
}