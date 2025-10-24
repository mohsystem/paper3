#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <cstdio> // For tmpfile, fprintf, fclose

// For unique temporary file names (more secure than fixed names)
#if defined(_WIN32)
#include <windows.h>
#include <fileapi.h>
#else
#include <unistd.h>
#include <cstdlib>
#endif

/**
 * Performs a binary search on a sorted vector of integers.
 *
 * @param arr The sorted vector to search in.
 * @param target The integer to search for.
 * @return The index of the target element, or -1 if it's not found.
 */
int binarySearch(const std::vector<int>& arr, int target) {
    if (arr.empty()) {
        return -1;
    }

    int low = 0;
    int high = arr.size() - 1;

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
 * @param arr The sorted vector to search in.
 * @param target The integer to search for.
 * @return The index of the target element, or -1 if not found.
 */
int measureAndWriteTime(const std::vector<int>& arr, int target) {
    auto start = std::chrono::high_resolution_clock::now();
    int result = binarySearch(arr, target);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    // Using tmpfile() for a secure, auto-deleting temporary file
    FILE* tempFile = std::tmpfile();

    if (tempFile == nullptr) {
        std::cerr << "Error: Unable to create temporary file." << std::endl;
        return result;
    }

    std::fprintf(tempFile, "Binary search for target %d took %lld nanoseconds.\n", target, duration.count());
    std::cout << "Execution time written to a temporary file." << std::endl;
    
    // The file is automatically deleted when closed.
    std::fclose(tempFile);

    return result;
}

int main() {
    // The vector must be sorted for binary search to work correctly.
    std::vector<int> sortedVector = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Element in the middle
    int target1 = 23;
    int result1 = measureAndWriteTime(sortedVector, target1);
    std::cout << "Target: " << target1 << ", Found at index: " << result1 << "\n" << std::endl;

    // Test Case 2: Element not present
    int target2 = 15;
    int result2 = measureAndWriteTime(sortedVector, target2);
    std::cout << "Target: " << target2 << ", Found at index: " << result2 << "\n" << std::endl;

    // Test Case 3: First element
    int target3 = 2;
    int result3 = measureAndWriteTime(sortedVector, target3);
    std::cout << "Target: " << target3 << ", Found at index: " << result3 << "\n" << std::endl;

    // Test Case 4: Last element
    int target4 = 91;
    int result4 = measureAndWriteTime(sortedVector, target4);
    std::cout << "Target: " << target4 << ", Found at index: " << result4 << "\n" << std::endl;

    // Test Case 5: Empty vector
    std::vector<int> emptyVector = {};
    int target5 = 5;
    int result5 = measureAndWriteTime(emptyVector, target5);
    std::cout << "Target: " << target5 << " (in empty vector), Found at index: " << result5 << "\n" << std::endl;

    return 0;
}