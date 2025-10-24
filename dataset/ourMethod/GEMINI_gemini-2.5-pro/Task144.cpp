#include <iostream>
#include <vector>
#include <algorithm> // For std::copy

/**
 * Merges two sorted integer vectors into a single sorted vector.
 *
 * @param arr1 The first sorted vector.
 * @param arr2 The second sorted vector.
 * @return A new vector containing all elements from arr1 and arr2 in sorted order.
 */
std::vector<int> mergeSortedArrays(const std::vector<int>& arr1, const std::vector<int>& arr2) {
    size_t n1 = arr1.size();
    size_t n2 = arr2.size();
    std::vector<int> mergedArray(n1 + n2);

    size_t i = 0, j = 0, k = 0;

    // Traverse both arrays and insert the smaller element into the merged array
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            mergedArray[k++] = arr1[i++];
        } else {
            mergedArray[k++] = arr2[j++];
        }
    }

    // Copy remaining elements of arr1, if any
    while (i < n1) {
        mergedArray[k++] = arr1[i++];
    }

    // Copy remaining elements of arr2, if any
    while (j < n2) {
        mergedArray[k++] = arr2[j++];
    }

    return mergedArray;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    // Test Case 1: General case
    std::vector<int> arr1_1 = {1, 3, 5};
    std::vector<int> arr1_2 = {2, 4, 6};
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "Input: "; printVector(arr1_1); std::cout << ", "; printVector(arr1_2); std::cout << std::endl;
    std::vector<int> res1 = mergeSortedArrays(arr1_1, arr1_2);
    std::cout << "Output: "; printVector(res1); std::cout << std::endl << std::endl;

    // Test Case 2: One array is empty
    std::vector<int> arr2_1 = {10, 20, 30};
    std::vector<int> arr2_2 = {};
    std::cout << "Test Case 2:" << std::endl;
    std::cout << "Input: "; printVector(arr2_1); std::cout << ", "; printVector(arr2_2); std::cout << std::endl;
    std::vector<int> res2 = mergeSortedArrays(arr2_1, arr2_2);
    std::cout << "Output: "; printVector(res2); std::cout << std::endl << std::endl;
    
    // Test Case 3: Both arrays are empty
    std::vector<int> arr3_1 = {};
    std::vector<int> arr3_2 = {};
    std::cout << "Test Case 3:" << std::endl;
    std::cout << "Input: "; printVector(arr3_1); std::cout << ", "; printVector(arr3_2); std::cout << std::endl;
    std::vector<int> res3 = mergeSortedArrays(arr3_1, arr3_2);
    std::cout << "Output: "; printVector(res3); std::cout << std::endl << std::endl;

    // Test Case 4: Arrays with different lengths
    std::vector<int> arr4_1 = {1, 2, 9, 15};
    std::vector<int> arr4_2 = {3, 10};
    std::cout << "Test Case 4:" << std::endl;
    std::cout << "Input: "; printVector(arr4_1); std::cout << ", "; printVector(arr4_2); std::cout << std::endl;
    std::vector<int> res4 = mergeSortedArrays(arr4_1, arr4_2);
    std::cout << "Output: "; printVector(res4); std::cout << std::endl << std::endl;

    // Test Case 5: Arrays with duplicate and negative numbers
    std::vector<int> arr5_1 = {-5, 0, 5};
    std::vector<int> arr5_2 = {-10, 0, 10};
    std::cout << "Test Case 5:" << std::endl;
    std::cout << "Input: "; printVector(arr5_1); std::cout << ", "; printVector(arr5_2); std::cout << std::endl;
    std::vector<int> res5 = mergeSortedArrays(arr5_1, arr5_2);
    std::cout << "Output: "; printVector(res5); std::cout << std::endl << std::endl;
    
    return 0;
}