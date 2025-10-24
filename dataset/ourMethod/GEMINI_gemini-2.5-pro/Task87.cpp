#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <filesystem> // Requires C++17

/**
 * Performs a binary search on a sorted vector of integers.
 *
 * @param arr    The sorted vector to search in.
 * @param target The value to search for.
 * @return The index of the target, or -1 if not found.
 */
int binarySearch(const std::vector<int>& arr, int target) {
    if (arr.empty()) {
        return -1;
    }
    
    int low = 0;
    // Note: Using int for high is safe here because vector::size() returns size_t,
    // which could be larger than int, but practical limits on array sizes make this fine.
    // For extremely large vectors, a 64-bit integer type would be safer.
    int high = static_cast<int>(arr.size() - 1);

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

void printArray(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    const std::vector<int> sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    const std::vector<int> testTargets = {23, 91, 2, 15, 100};

    try {
        std::filesystem::path tempFilePath = std::filesystem::temp_directory_path() / "task87_cpp_times.txt";
        std::cout << "Execution times will be written to: " << tempFilePath << std::endl;
        
        // std::ofstream's constructor opens the file, and its destructor will close it (RAII).
        std::ofstream outFile(tempFilePath);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open temporary file for writing." << std::endl;
            return 1;
        }

        for (int target : testTargets) {
            auto start = std::chrono::high_resolution_clock::now();
            int result = binarySearch(sortedArray, target);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

            std::cout << "Array: ";
            printArray(sortedArray);
            std::cout << std::endl;
            std::cout << "Searching for: " << target << std::endl;
            if (result != -1) {
                std::cout << "Found at index: " << result << std::endl;
            } else {
                std::cout << "Not found." << std::endl;
            }
            std::cout << "Execution time: " << duration.count() << " nanoseconds.\n" << std::endl;

            outFile << "Target: " << target << ", Time (ns): " << duration.count() << "\n";
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}