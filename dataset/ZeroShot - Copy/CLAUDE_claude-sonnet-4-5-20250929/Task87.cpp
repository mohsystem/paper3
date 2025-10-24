
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>

int binarySearch(const std::vector<int>& arr, int target) {
    int left = 0;
    int right = arr.size() - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

void writeExecutionTime(long long executionTime) {
    char tempFileName[L_tmpnam];
    std::tmpnam(tempFileName);
    std::string fileName = std::string(tempFileName) + "_execution_time.txt";
    
    std::ofstream tempFile(fileName);
    if (tempFile.is_open()) {
        tempFile << "Binary Search Execution Time: " << executionTime << " nanoseconds\\n";
        tempFile.close();
        std::cout << "Execution time written to: " << fileName << std::endl;
    } else {
        std::cerr << "Error writing to file" << std::endl;
    }
}

void printArray(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << arr[i];
        if (i < arr.size() - 1) std::cout << ", ";
    }
    std::cout << "]";
}

int main() {
    // Test case 1
    std::vector<int> arr1 = {1, 3, 5, 7, 9, 11, 13, 15};
    int target1 = 7;
    auto start1 = std::chrono::high_resolution_clock::now();
    int result1 = binarySearch(arr1, target1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count();
    std::cout << "Test 1: Array: ";
    printArray(arr1);
    std::cout << ", Target: " << target1 << ", Result: " << result1 << std::endl;
    writeExecutionTime(duration1);
    
    // Test case 2
    std::vector<int> arr2 = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    int target2 = 12;
    auto start2 = std::chrono::high_resolution_clock::now();
    int result2 = binarySearch(arr2, target2);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count();
    std::cout << "Test 2: Array: ";
    printArray(arr2);
    std::cout << ", Target: " << target2 << ", Result: " << result2 << std::endl;
    writeExecutionTime(duration2);
    
    // Test case 3
    std::vector<int> arr3 = {1, 2, 3, 4, 5};
    int target3 = 6;
    auto start3 = std::chrono::high_resolution_clock::now();
    int result3 = binarySearch(arr3, target3);
    auto end3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::nanoseconds>(end3 - start3).count();
    std::cout << "Test 3: Array: ";
    printArray(arr3);
    std::cout << ", Target: " << target3 << ", Result: " << result3 << std::endl;
    writeExecutionTime(duration3);
    
    // Test case 4
    std::vector<int> arr4 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int target4 = 10;
    auto start4 = std::chrono::high_resolution_clock::now();
    int result4 = binarySearch(arr4, target4);
    auto end4 = std::chrono::high_resolution_clock::now();
    auto duration4 = std::chrono::duration_cast<std::chrono::nanoseconds>(end4 - start4).count();
    std::cout << "Test 4: Array: ";
    printArray(arr4);
    std::cout << ", Target: " << target4 << ", Result: " << result4 << std::endl;
    writeExecutionTime(duration4);
    
    // Test case 5
    std::vector<int> arr5 = {5, 15, 25, 35, 45, 55};
    int target5 = 55;
    auto start5 = std::chrono::high_resolution_clock::now();
    int result5 = binarySearch(arr5, target5);
    auto end5 = std::chrono::high_resolution_clock::now();
    auto duration5 = std::chrono::duration_cast<std::chrono::nanoseconds>(end5 - start5).count();
    std::cout << "Test 5: Array: ";
    printArray(arr5);
    std::cout << ", Target: " << target5 << ", Result: " << result5 << std::endl;
    writeExecutionTime(duration5);
    
    return 0;
}
