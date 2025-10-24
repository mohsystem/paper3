#include <iostream>
#include <vector>

/**
 * Merges two sorted integer vectors into a single sorted vector.
 *
 * @param arr1 The first sorted integer vector.
 * @param arr2 The second sorted integer vector.
 * @return A new sorted vector containing all elements from arr1 and arr2.
 */
std::vector<int> mergeArrays(const std::vector<int>& arr1, const std::vector<int>& arr2) {
    if (arr1.empty()) {
        return arr2;
    }
    if (arr2.empty()) {
        return arr1;
    }

    size_t n1 = arr1.size();
    size_t n2 = arr2.size();
    std::vector<int> mergedArray(n1 + n2);

    size_t i = 0; // Pointer for arr1
    size_t j = 0; // Pointer for arr2
    size_t k = 0; // Pointer for mergedArray

    // Traverse both vectors and insert the smaller element into the merged vector
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

// Helper function to print a vector
void printVector(const std::string& name, const std::vector<int>& arr) {
    std::cout << name;
    for (int x : arr) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Standard merge
    std::vector<int> arr1 = {1, 3, 5, 7};
    std::vector<int> arr2 = {2, 4, 6, 8};
    std::cout << "Test Case 1:" << std::endl;
    printVector("Array 1: ", arr1);
    printVector("Array 2: ", arr2);
    std::vector<int> merged1 = mergeArrays(arr1, arr2);
    printVector("Merged:  ", merged1);
    std::cout << std::endl;

    // Test Case 2: Arrays with different lengths
    std::vector<int> arr3 = {10, 20, 30};
    std::vector<int> arr4 = {5, 15, 25, 35, 45};
    std::cout << "Test Case 2:" << std::endl;
    printVector("Array 1: ", arr3);
    printVector("Array 2: ", arr4);
    std::vector<int> merged2 = mergeArrays(arr3, arr4);
    printVector("Merged:  ", merged2);
    std::cout << std::endl;

    // Test Case 3: One array is empty
    std::vector<int> arr5 = {1, 2, 3};
    std::vector<int> arr6 = {};
    std::cout << "Test Case 3:" << std::endl;
    printVector("Array 1: ", arr5);
    printVector("Array 2: ", arr6);
    std::vector<int> merged3 = mergeArrays(arr5, arr6);
    printVector("Merged:  ", merged3);
    std::cout << std::endl;

    // Test Case 4: Both arrays are empty
    std::vector<int> arr7 = {};
    std::vector<int> arr8 = {};
    std::cout << "Test Case 4:" << std::endl;
    printVector("Array 1: ", arr7);
    printVector("Array 2: ", arr8);
    std::vector<int> merged4 = mergeArrays(arr7, arr8);
    printVector("Merged:  ", merged4);
    std::cout << std::endl;

    // Test Case 5: Arrays with negative numbers and duplicates
    std::vector<int> arr9 = {-5, 0, 5, 5, 15};
    std::vector<int> arr10 = {-10, -5, 10, 20};
    std::cout << "Test Case 5:" << std::endl;
    printVector("Array 1: ", arr9);
    printVector("Array 2: ", arr10);
    std::vector<int> merged5 = mergeArrays(arr9, arr10);
    printVector("Merged:  ", merged5);
    std::cout << std::endl;

    return 0;
}