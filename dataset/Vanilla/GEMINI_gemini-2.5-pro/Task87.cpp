#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

/**
 * Performs binary search on a sorted vector.
 *
 * @param arr The sorted vector to search in.
 * @param target The element to search for.
 * @return The index of the target element, or -1 if not found.
 */
int binarySearch(const std::vector<int>& arr, int target) {
    int left = 0;
    int right = arr.size() - 1;

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
    std::ofstream outFile("cpp_execution_log.tmp", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "Target: " << target << ", Execution Time: " << duration << " ns\n";
    } else {
        std::cerr << "Error writing to temporary file." << std::endl;
    }
}

void printVector(const std::vector<int>& vec) {
    std::cout << "Array: [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // The vector must be sorted for binary search to work.
    std::vector<int> sortedVector = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    std::vector<int> testCases = {23, 91, 2, 15, 100}; // Targets to search for

    std::cout << "Starting C++ Binary Search Test Cases..." << std::endl;
    printVector(sortedVector);

    for (int target : testCases) {
        std::cout << "----------------------------------------" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(sortedVector, target);
        auto end = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        std::cout << "Searching for: " << target << std::endl;
        if (result != -1) {
            std::cout << "Element found at index: " << result << std::endl;
        } else {
            std::cout << "Element not found." << std::endl;
        }
        std::cout << "Execution time: " << duration << " nanoseconds." << std::endl;

        // Write the execution time to a temporary file
        writeExecutionTime(duration, target);
    }
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "All execution times have been logged to cpp_execution_log.tmp" << std::endl;

    return 0;
}