
#include <iostream>
#include <vector>
#include <algorithm>

std::vector<int> sortArray(std::vector<int> arr) {
    if (arr.empty()) {
        return arr;
    }
    std::vector<int> result = arr;
    std::sort(result.begin(), result.end());
    return result;
}

void printArray(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << arr[i];
        if (i < arr.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test case 1: Regular array
    std::vector<int> test1 = {5, 2, 8, 1, 9};
    std::cout << "Test 1: ";
    printArray(sortArray(test1));
    
    // Test case 2: Already sorted array
    std::vector<int> test2 = {1, 2, 3, 4, 5};
    std::cout << "Test 2: ";
    printArray(sortArray(test2));
    
    // Test case 3: Reverse sorted array
    std::vector<int> test3 = {9, 7, 5, 3, 1};
    std::cout << "Test 3: ";
    printArray(sortArray(test3));
    
    // Test case 4: Array with duplicates
    std::vector<int> test4 = {4, 2, 7, 2, 9, 4};
    std::cout << "Test 4: ";
    printArray(sortArray(test4));
    
    // Test case 5: Single element array
    std::vector<int> test5 = {42};
    std::cout << "Test 5: ";
    printArray(sortArray(test5));
    
    return 0;
}
