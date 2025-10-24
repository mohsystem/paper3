#include <iostream>
#include <vector>

int findOdd(const std::vector<int>& arr) {
    int result = 0;
    for (int number : arr) {
        result ^= number;
    }
    return result;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {7};
    std::cout << "Test Case 1: ";
    printVector(arr1);
    std::cout << " -> " << findOdd(arr1) << std::endl;
    
    // Test Case 2
    std::vector<int> arr2 = {0};
    std::cout << "Test Case 2: ";
    printVector(arr2);
    std::cout << " -> " << findOdd(arr2) << std::endl;
    
    // Test Case 3
    std::vector<int> arr3 = {1, 1, 2};
    std::cout << "Test Case 3: ";
    printVector(arr3);
    std::cout << " -> " << findOdd(arr3) << std::endl;
    
    // Test Case 4
    std::vector<int> arr4 = {0, 1, 0, 1, 0};
    std::cout << "Test Case 4: ";
    printVector(arr4);
    std::cout << " -> " << findOdd(arr4) << std::endl;
    
    // Test Case 5
    std::vector<int> arr5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    std::cout << "Test Case 5: ";
    printVector(arr5);
    std::cout << " -> " << findOdd(arr5) << std::endl;
    
    return 0;
}