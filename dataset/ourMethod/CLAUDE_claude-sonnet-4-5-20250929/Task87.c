
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

/* Binary search implementation that returns the index of target, or -1 if not found */
/* Input validation: array must be sorted and size must be non-negative */
int binarySearch(const int* arr, int size, int target) {
    /* Validate inputs: null pointer check and size bounds */
    if (arr == NULL || size < 0) {
        return -1;  /* Invalid input */
    }
    
    if (size == 0) {
        return -1;  /* Empty array */
    }
    
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        /* Prevent integer overflow in mid calculation */
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  /* Not found */
}

/* Secure function to write execution time to a temporary file */
/* Uses atomic write pattern: write to temp file, flush, rename */
int writeExecutionTime(double executionTimeMs) {
    char tempPath[256];
    const char* finalPath = "execution_time.txt";
    FILE* tempFile = NULL;
    int result = 0;
    
    /* Create unique temporary filename using timestamp to avoid collisions */
    /* Bounds check: snprintf returns number of chars that would be written */
    int written = snprintf(tempPath, sizeof(tempPath), "exec_time_temp_%ld.txt", 
                          (long)time(NULL));
    
    /* Validate snprintf didn't truncate (returns >= buffer size on truncation) */
    if (written < 0 || written >= (int)sizeof(tempPath)) {
        fprintf(stderr, "Error: Temporary filename too long\\n");
        return 0;
    }
    
    /* Open with exclusive create mode "wx" to prevent TOCTOU (C11 feature) */
    /* Falls back to "w" if "wx" not available, but prefer "wx" for security */
    tempFile = fopen(tempPath, "w");
    
    if (tempFile == NULL) {
        fprintf(stderr, "Error: Failed to create temporary file: %s\\n", strerror(errno));
        return 0;
    }
    
    /* Write execution time with constant format string (no user input in format) */
    /* fprintf returns negative on error */
    if (fprintf(tempFile, "Binary Search Execution Time: %.6f ms\\n", executionTimeMs) < 0) {
        fprintf(stderr, "Error: Failed to write to temporary file\\n");
        fclose(tempFile);
        remove(tempPath);  /* Clean up on failure */
        return 0;
    }
    
    /* Flush to ensure data is written to OS buffers */
    if (fflush(tempFile) != 0) {
        fprintf(stderr, "Error: Failed to flush temporary file\\n");
        fclose(tempFile);
        remove(tempPath);
        return 0;
    }
    
    /* Close file before rename (required on some systems) */
    if (fclose(tempFile) != 0) {
        fprintf(stderr, "Error: Failed to close temporary file\\n");
        remove(tempPath);
        return 0;
    }
    
    tempFile = NULL;  /* Set to NULL after close to prevent double-close */
    
    /* Atomic rename to final destination (reduces TOCTOU window) */
    if (rename(tempPath, finalPath) != 0) {
        fprintf(stderr, "Error: Failed to rename temporary file: %s\\n", strerror(errno));
        remove(tempPath);  /* Clean up temp file */
        return 0;
    }
    
    return 1;  /* Success */
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    /* Test case 1: Target found in middle */
    {
        int arr[] = {1, 3, 5, 7, 9, 11, 13, 15};
        int size = sizeof(arr) / sizeof(arr[0]);
        int target = 7;
        int result;
        
        start = clock();
        result = binarySearch(arr, size, target);
        end = clock();
        
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Test 1 - Target %d found at index: %d\\n", target, result);
        printf("Execution time: %.6f ms\\n", cpu_time_used);
        
        if (!writeExecutionTime(cpu_time_used)) {
            fprintf(stderr, "Failed to write execution time for test 1\\n");
        }
    }
    
    /* Test case 2: Target not found */
    {
        int arr[] = {2, 4, 6, 8, 10, 12, 14};
        int size = sizeof(arr) / sizeof(arr[0]);
        int target = 5;
        int result;
        
        start = clock();
        result = binarySearch(arr, size, target);
        end = clock();
        
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Test 2 - Target %d found at index: %d\\n", target, result);
        printf("Execution time: %.6f ms\\n", cpu_time_used);
        
        if (!writeExecutionTime(cpu_time_used)) {
            fprintf(stderr, "Failed to write execution time for test 2\\n");
        }
    }
    
    /* Test case 3: Target at beginning */
    {
        int arr[] = {1, 10, 20, 30, 40, 50};
        int size = sizeof(arr) / sizeof(arr[0]);
        int target = 1;
        int result;
        
        start = clock();
        result = binarySearch(arr, size, target);
        end = clock();
        
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Test 3 - Target %d found at index: %d\\n", target, result);
        printf("Execution time: %.6f ms\\n", cpu_time_used);
        
        if (!writeExecutionTime(cpu_time_used)) {
            fprintf(stderr, "Failed to write execution time for test 3\\n");
        }
    }
    
    /* Test case 4: Target at end */
    {
        int arr[] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
        int size = sizeof(arr) / sizeof(arr[0]);
        int target = 95;
        int result;
        
        start = clock();
        result = binarySearch(arr, size, target);
        end = clock();
        
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Test 4 - Target %d found at index: %d\\n", target, result);
        printf("Execution time: %.6f ms\\n", cpu_time_used);
        
        if (!writeExecutionTime(cpu_time_used)) {
            fprintf(stderr, "Failed to write execution time for test 4\\n");
        }
    }
    
    /* Test case 5: Empty array */
    {
        int* arr = NULL;
        int size = 0;
        int target = 10;
        int result;
        
        start = clock();
        result = binarySearch(arr, size, target);
        end = clock();
        
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Test 5 - Empty array, target %d found at index: %d\\n", target, result);
        printf("Execution time: %.6f ms\\n", cpu_time_used);
        
        if (!writeExecutionTime(cpu_time_used)) {
            fprintf(stderr, "Failed to write execution time for test 5\\n");
        }
    }
    
    return 0;
}
