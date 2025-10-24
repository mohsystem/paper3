#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// For high-resolution timer
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <time.h>
#else
#include <time.h> // Fallback to clock()
#endif


/**
 * Performs a binary search on a sorted array of integers.
 *
 * @param arr The sorted array to search in. Must not be NULL.
 * @param size The number of elements in the array.
 * @param target The integer to search for.
 * @return The index of the target element, or -1 if it's not found or if the array is invalid.
 */
int binary_search(const int* arr, size_t size, int target) {
    if (arr == NULL || size == 0) {
        return -1;
    }

    int low = 0;
    int high = size - 1;

    while (low <= high) {
        // Secure way to calculate mid to prevent potential overflow
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
 * Measures the execution time of the binary search and writes it to a secure temporary file.
 *
 * @param arr The sorted array to search in.
 * @param size The number of elements in the array.
 * @param target The integer to search for.
 * @return The index of the target element, or -1 if not found.
 */
int measure_and_write_time(const int* arr, size_t size, int target) {
    long long duration_ns;

#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#else
    clock_t start, end;
    start = clock();
#endif
    
    int result = binary_search(arr, size, target);

#if defined(_WIN32)
    QueryPerformanceCounter(&end);
    duration_ns = (end.QuadPart - start.QuadPart) * 1000000000LL / frequency.QuadPart;
#elif defined(__linux__) || defined(__APPLE__)
    clock_gettime(CLOCK_MONOTONIC, &end);
    duration_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
#else
    end = clock();
    duration_ns = (long long)(((double)(end - start) / CLOCKS_PER_SEC) * 1000000000.0);
#endif


    // Use tmpfile() for a secure, auto-deleting temporary file.
    // It returns a file stream that is automatically removed on fclose or program termination.
    FILE* temp_file = tmpfile();

    if (temp_file == NULL) {
        perror("Error creating temporary file");
        return result;
    }
    
    fprintf(temp_file, "Binary search for target %d took %lld nanoseconds.\n", target, duration_ns);
    printf("Execution time written to a temporary file.\n");

    // Close the file, which also deletes it.
    fclose(temp_file);

    return result;
}

int main() {
    // The array must be sorted for binary search to work correctly.
    int sorted_array[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    size_t size = sizeof(sorted_array) / sizeof(sorted_array[0]);

    printf("--- C Test Cases ---\n");

    // Test Case 1: Element in the middle
    int target1 = 23;
    int result1 = measure_and_write_time(sorted_array, size, target1);
    printf("Target: %d, Found at index: %d\n\n", target1, result1);

    // Test Case 2: Element not present
    int target2 = 15;
    int result2 = measure_and_write_time(sorted_array, size, target2);
    printf("Target: %d, Found at index: %d\n\n", target2, result2);

    // Test Case 3: First element
    int target3 = 2;
    int result3 = measure_and_write_time(sorted_array, size, target3);
    printf("Target: %d, Found at index: %d\n\n", target3, result3);

    // Test Case 4: Last element
    int target4 = 91;
    int result4 = measure_and_write_time(sorted_array, size, target4);
    printf("Target: %d, Found at index: %d\n\n", target4, result4);

    // Test Case 5: Empty array
    int* empty_array = NULL;
    int target5 = 5;
    int result5 = measure_and_write_time(empty_array, 0, target5);
    printf("Target: %d (in empty array), Found at index: %d\n\n", target5, result5);

    return 0;
}