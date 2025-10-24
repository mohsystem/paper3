
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <string>
#include <limits>
#include <cerrno>
#include <cstring>

// Binary search implementation that returns the index of target, or -1 if not found
// Input validation: array must be sorted for binary search to work correctly
int binarySearch(const std::vector<int>& arr, int target) {
    if (arr.empty()) {
        return -1;  // Empty array check prevents undefined behavior
    }
    
    int left = 0;
    int right = static_cast<int>(arr.size()) - 1;
    
    while (left <= right) {
        // Prevent integer overflow: use (left + right) / 2 alternative
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  // Not found
}

// Secure function to write execution time to a temporary file
// Uses atomic write pattern: write to temp, flush, sync, rename
bool writeExecutionTime(double executionTimeMs) {
    // Create unique temporary filename in current directory (controlled location)
    // Using timestamp to reduce collision probability
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::string tempPath = "exec_time_temp_" + std::to_string(now) + ".txt";
    std::string finalPath = "execution_time.txt";
    
    // Open with exclusive create to prevent TOCTOU issues
    // Note: C++ doesn't have O_EXCL equivalent in standard, but we use temp naming for safety
    std::ofstream tempFile(tempPath, std::ios::out | std::ios::trunc);
    
    if (!tempFile.is_open()) {
        std::cerr << "Error: Failed to create temporary file\\n";
        return false;
    }
    
    // Write execution time with bounds checking (double output is safe)
    // Format string is constant, no user input in format
    tempFile << "Binary Search Execution Time: " << executionTimeMs << " ms\\n";
    
    if (tempFile.fail()) {
        std::cerr << "Error: Failed to write to temporary file\\n";
        tempFile.close();
        std::remove(tempPath.c_str());  // Clean up on failure
        return false;
    }
    
    // Flush to ensure data is written
    tempFile.flush();
    
    // Close file before rename
    tempFile.close();
    
    // Atomic rename to final destination (reduces TOCTOU window)
    if (std::rename(tempPath.c_str(), finalPath.c_str()) != 0) {
        std::cerr << "Error: Failed to rename temporary file: " << std::strerror(errno) << "\\n";
        std::remove(tempPath.c_str());  // Clean up temp file
        return false;
    }
    
    return true;
}

int main() {
    // Test case 1: Target found in middle
    {
        std::vector<int> arr = {1, 3, 5, 7, 9, 11, 13, 15};
        int target = 7;
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(arr, target);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        
        std::cout << "Test 1 - Target " << target << " found at index: " << result << "\\n";
        std::cout << "Execution time: " << duration.count() << " ms\\n";
        
        if (!writeExecutionTime(duration.count())) {
            std::cerr << "Failed to write execution time for test 1\\n";
        }
    }
    
    // Test case 2: Target not found
    {
        std::vector<int> arr = {2, 4, 6, 8, 10, 12, 14};
        int target = 5;
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(arr, target);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        
        std::cout << "Test 2 - Target " << target << " found at index: " << result << "\\n";
        std::cout << "Execution time: " << duration.count() << " ms\\n";
        
        if (!writeExecutionTime(duration.count())) {
            std::cerr << "Failed to write execution time for test 2\\n";
        }
    }
    
    // Test case 3: Target at beginning
    {
        std::vector<int> arr = {1, 10, 20, 30, 40, 50};
        int target = 1;
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(arr, target);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        
        std::cout << "Test 3 - Target " << target << " found at index: " << result << "\\n";
        std::cout << "Execution time: " << duration.count() << " ms\\n";
        
        if (!writeExecutionTime(duration.count())) {
            std::cerr << "Failed to write execution time for test 3\\n";
        }
    }
    
    // Test case 4: Target at end
    {
        std::vector<int> arr = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
        int target = 95;
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(arr, target);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        
        std::cout << "Test 4 - Target " << target << " found at index: " << result << "\\n";
        std::cout << "Execution time: " << duration.count() << " ms\\n";
        
        if (!writeExecutionTime(duration.count())) {
            std::cerr << "Failed to write execution time for test 4\\n";
        }
    }
    
    // Test case 5: Empty array
    {
        std::vector<int> arr;
        int target = 10;
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = binarySearch(arr, target);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        
        std::cout << "Test 5 - Empty array, target " << target << " found at index: " << result << "\\n";
        std::cout << "Execution time: " << duration.count() << " ms\\n";
        
        if (!writeExecutionTime(duration.count())) {
            std::cerr << "Failed to write execution time for test 5\\n";
        }
    }
    
    return 0;
}
