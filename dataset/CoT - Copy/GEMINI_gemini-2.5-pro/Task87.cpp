#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>

/**
 * @brief Performs a binary search on a sorted vector of integers.
 * This implementation assumes the input vector is sorted.
 * 
 * @param arr The sorted vector to search in.
 * @param target The integer to search for.
 * @return The index of the target, or -1 if not found.
 */
int binarySearch(const std::vector<int>& arr, int target) {
    if (arr.empty()) {
        return -1;
    }

    int low = 0;
    int high = arr.size() - 1;

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
 * @brief Executes binary search, measures its time, and logs it to a file.
 * NOTE: This function writes to a fixed file named "cpp_execution_times.log"
 * in the current directory for simplicity and portability, as creating secure,
 * cross-platform temporary files in C++ before C++17 is complex.
 * 
 * @param arr The sorted vector to search in.
 * @param target The integer to search for.
 * @return The index of the target, or -1 if not found.
 */
int performSearchAndLogTime(const std::vector<int>& arr, int target) {
    auto start = std::chrono::high_resolution_clock::now();
    int result = binarySearch(arr, target);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    const char* filename = "cpp_execution_times.log";
    // Append to the log file
    std::ofstream log_file(filename, std::ios_base::app);

    if (log_file.is_open()) {
        log_file << "Binary search for target " << target << " took " << duration << " nanoseconds. Result index: " << result << std::endl;
        log_file.close(); // RAII would handle this, but explicit close is fine
        std::cout << "Execution time appended to log file: " << filename << std::endl;
    } else {
        std::cerr << "Error: Unable to open log file " << filename << std::endl;
    }

    return result;
}

int main() {
    // The vector must be sorted for binary search to work correctly.
    std::vector<int> sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    
    std::cout << "Running test cases..." << std::endl;

    // Test Case 1: Element in the middle
    int target1 = 23;
    std::cout << "Searching for " << target1 << "... Found at index: " << performSearchAndLogTime(sortedArray, target1) << std::endl << std::endl;

    // Test Case 2: Element at the beginning
    int target2 = 2;
    std::cout << "Searching for " << target2 << "... Found at index: " << performSearchAndLogTime(sortedArray, target2) << std::endl << std::endl;

    // Test Case 3: Element at the end
    int target3 = 91;
    std::cout << "Searching for " << target3 << "... Found at index: " << performSearchAndLogTime(sortedArray, target3) << std::endl << std::endl;

    // Test Case 4: Element not found (smaller than all)
    int target4 = 1;
    std::cout << "Searching for " << target4 << "... Found at index: " << performSearchAndLogTime(sortedArray, target4) << std::endl << std::endl;

    // Test Case 5: Element not found (larger than all)
    int target5 = 100;
    std::cout << "Searching for " << target5 << "... Found at index: " << performSearchAndLogTime(sortedArray, target5) << std::endl << std::endl;
    
    return 0;
}