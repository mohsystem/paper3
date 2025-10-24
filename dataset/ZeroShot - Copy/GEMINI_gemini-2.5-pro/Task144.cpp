#include <iostream>
#include <vector>
#include <limits> // For std::numeric_limits

/**
 * Merges two sorted vectors of integers into a single sorted vector.
 * This function is secure against size_t overflow for the combined vector size.
 *
 * @param arr1 A constant reference to the first sorted vector.
 * @param arr2 A constant reference to the second sorted vector.
 * @return A new sorted vector containing all elements from arr1 and arr2.
 *         Returns an empty vector on size overflow.
 */
std::vector<int> mergeSortedArrays(const std::vector<int>& arr1, const std::vector<int>& arr2) {
    size_t size1 = arr1.size();
    size_t size2 = arr2.size();
    
    // Security: Check for size_t overflow before calculating total size
    if (size1 > std::numeric_limits<size_t>::max() - size2) {
        std::cerr << "Error: Combined vector size is too large, potential overflow." << std::endl;
        return {}; // Return empty vector to indicate failure
    }
    
    size_t totalSize = size1 + size2;
    if (totalSize == 0) {
        return {};
    }

    std::vector<int> merged(totalSize);
    
    size_t i = 0, j = 0, k = 0;

    // Traverse both vectors and insert the smaller element into the merged vector
    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            merged[k++] = arr1[i++];
        } else {
            merged[k++] = arr2[j++];
        }
    }

    // Copy remaining elements from arr1, if any
    while (i < size1) {
        merged[k++] = arr1[i++];
    }

    // Copy remaining elements from arr2, if any
    while (j < size2) {
        merged[k++] = arr2[j++];
    }

    return merged;
}

// Helper function to print a vector for testing
void printVector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Standard merge
    std::vector<int> arr1_1 = {1, 3, 5, 7};
    std::vector<int> arr1_2 = {2, 4, 6, 8};
    std::cout << "Test Case 1:" << std::endl;
    printVector("Array 1: ", arr1_1);
    printVector("Array 2: ", arr1_2);
    std::vector<int> merged1 = mergeSortedArrays(arr1_1, arr1_2);
    printVector("Merged:  ", merged1);
    std::cout << std::endl;

    // Test Case 2: One array is empty
    std::vector<int> arr2_1 = {10, 20, 30};
    std::vector<int> arr2_2 = {};
    std::cout << "Test Case 2:" << std::endl;
    printVector("Array 1: ", arr2_1);
    printVector("Array 2: ", arr2_2);
    std::vector<int> merged2 = mergeSortedArrays(arr2_1, arr2_2);
    printVector("Merged:  ", merged2);
    std::cout << std::endl;

    // Test Case 3: First array is empty
    std::vector<int> arr3_1 = {};
    std::vector<int> arr3_2 = {5, 15, 25};
    std::cout << "Test Case 3:" << std::endl;
    printVector("Array 1: ", arr3_1);
    printVector("Array 2: ", arr3_2);
    std::vector<int> merged3 = mergeSortedArrays(arr3_1, arr3_2);
    printVector("Merged:  ", merged3);
    std::cout << std::endl;

    // Test Case 4: Arrays with duplicate values and different lengths
    std::vector<int> arr4_1 = {1, 2, 2, 5, 9};
    std::vector<int> arr4_2 = {2, 6, 10};
    std::cout << "Test Case 4:" << std::endl;
    printVector("Array 1: ", arr4_1);
    printVector("Array 2: ", arr4_2);
    std::vector<int> merged4 = mergeSortedArrays(arr4_1, arr4_2);
    printVector("Merged:  ", merged4);
    std::cout << std::endl;

    // Test Case 5: Both arrays are empty
    std::vector<int> arr5_1 = {};
    std::vector<int> arr5_2 = {};
    std::cout << "Test Case 5:" << std::endl;
    printVector("Array 1: ", arr5_1);
    printVector("Array 2: ", arr5_2);
    std::vector<int> merged5 = mergeSortedArrays(arr5_1, arr5_2);
    printVector("Merged:  ", merged5);
    std::cout << std::endl;

    return 0;
}