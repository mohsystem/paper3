
#include <iostream>
#include <vector>
#include <algorithm>

std::vector<int> posNegSort(std::vector<int> arr) {
    if (arr.empty()) {
        return std::vector<int>();
    }
    
    // Extract positive numbers and sort them
    std::vector<int> positives;
    for (int num : arr) {
        if (num > 0) {
            positives.push_back(num);
        }
    }
    std::sort(positives.begin(), positives.end());
    
    // Create result vector and place sorted positives back
    std::vector<int> result;
    int posIndex = 0;
    
    for (int num : arr) {
        if (num < 0) {
            result.push_back(num);
        } else {
            result.push_back(positives[posIndex]);
            posIndex++;
        }
    }
    
    return result;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {6, 3, -2, 5, -8, 2, -2};
    std::cout << "Test 1: ";
    printVector(posNegSort(test1));
    
    // Test case 2
    std::vector<int> test2 = {6, 5, 4, -1, 3, 2, -1, 1};
    std::cout << "Test 2: ";
    printVector(posNegSort(test2));
    
    // Test case 3
    std::vector<int> test3 = {-5, -5, -5, -5, 7, -5};
    std::cout << "Test 3: ";
    printVector(posNegSort(test3));
    
    // Test case 4
    std::vector<int> test4 = {};
    std::cout << "Test 4: ";
    printVector(posNegSort(test4));
    
    // Test case 5
    std::vector<int> test5 = {10, -3, 5, -1, 8, -7, 3};
    std::cout << "Test 5: ";
    printVector(posNegSort(test5));
    
    return 0;
}
