#include <iostream>
#include <vector>
#include <algorithm>

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

std::vector<int> posNegSort(std::vector<int> arr) {
    // 1. Extract positive numbers
    std::vector<int> positives;
    for (int num : arr) {
        if (num > 0) {
            positives.push_back(num);
        }
    }

    // 2. Sort the positive numbers
    std::sort(positives.begin(), positives.end());

    // 3. Place sorted positives back into the array
    int posIndex = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] > 0) {
            arr[i] = positives[posIndex++];
        }
    }

    return arr;
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {6, 3, -2, 5, -8, 2, -2};
    std::cout << "Test 1 Input: "; printVector(arr1); std::cout << std::endl;
    std::vector<int> res1 = posNegSort(arr1);
    std::cout << "Test 1 Output: "; printVector(res1); std::cout << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {6, 5, 4, -1, 3, 2, -1, 1};
    std::cout << "Test 2 Input: "; printVector(arr2); std::cout << std::endl;
    std::vector<int> res2 = posNegSort(arr2);
    std::cout << "Test 2 Output: "; printVector(res2); std::cout << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {-5, -5, -5, -5, 7, -5};
    std::cout << "Test 3 Input: "; printVector(arr3); std::cout << std::endl;
    std::vector<int> res3 = posNegSort(arr3);
    std::cout << "Test 3 Output: "; printVector(res3); std::cout << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {};
    std::cout << "Test 4 Input: "; printVector(arr4); std::cout << std::endl;
    std::vector<int> res4 = posNegSort(arr4);
    std::cout << "Test 4 Output: "; printVector(res4); std::cout << std::endl;

    // Test Case 5
    std::vector<int> arr5 = {-1, -2, -3, 1, 2, 3};
    std::cout << "Test 5 Input: "; printVector(arr5); std::cout << std::endl;
    std::vector<int> res5 = posNegSort(arr5);
    std::cout << "Test 5 Output: "; printVector(res5); std::cout << std::endl;

    return 0;
}