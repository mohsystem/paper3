#include <iostream>
#include <vector>
#include <string>

/**
 * Merges two sorted integer vectors into a single sorted vector.
 *
 * @param arr1 The first sorted vector of integers.
 * @param arr2 The second sorted vector of integers.
 * @return A new vector containing all elements from arr1 and arr2 in sorted order.
 */
std::vector<int> mergeArrays(const std::vector<int>& arr1, const std::vector<int>& arr2) {
    int n1 = arr1.size();
    int n2 = arr2.size();
    std::vector<int> result(n1 + n2);

    int i = 0, j = 0, k = 0;

    // Traverse both arrays and insert the smaller element into the result
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    // Copy remaining elements of arr1, if any
    while (i < n1) {
        result[k++] = arr1[i++];
    }

    // Copy remaining elements of arr2, if any
    while (j < n2) {
        result[k++] = arr2[j++];
    }

    return result;
}

void printVector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1
    std::vector<int> arr1_1 = {1, 3, 5};
    std::vector<int> arr1_2 = {2, 4, 6};
    printVector("Test Case 1: ", mergeArrays(arr1_1, arr1_2));

    // Test Case 2
    std::vector<int> arr2_1 = {10, 20, 30};
    std::vector<int> arr2_2 = {};
    printVector("Test Case 2: ", mergeArrays(arr2_1, arr2_2));
    
    // Test Case 3
    std::vector<int> arr3_1 = {};
    std::vector<int> arr3_2 = {5, 15, 25};
    printVector("Test Case 3: ", mergeArrays(arr3_1, arr3_2));

    // Test Case 4
    std::vector<int> arr4_1 = {1, 5, 9, 10};
    std::vector<int> arr4_2 = {2, 3, 8};
    printVector("Test Case 4: ", mergeArrays(arr4_1, arr4_2));

    // Test Case 5
    std::vector<int> arr5_1 = {1, 2, 3};
    std::vector<int> arr5_2 = {4, 5, 6};
    printVector("Test Case 5: ", mergeArrays(arr5_1, arr5_2));
    
    return 0;
}