
#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>

// Security: Validate array size to prevent integer overflow and excessive memory usage
const size_t MAX_ARRAY_SIZE = 1000000;

// Heapify a subtree rooted at index i
// Security: All array accesses are bounds-checked via size parameter
void heapify(std::vector<int>& arr, size_t n, size_t i) {
    // Security: Prevent potential overflow in index calculations
    if (i >= n) return;
    
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;
    
    // Security: Bounds checking before array access
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }
    
    // Security: Bounds checking before array access
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }
    
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

// Main heap sort function
// Security: Uses vector for automatic memory management (RAII)
void heapSort(std::vector<int>& arr) {
    size_t n = arr.size();
    
    // Security: Handle empty or single-element arrays safely
    if (n <= 1) return;
    
    // Build max heap
    // Security: Loop bounds checked to prevent underflow (using size_t carefully)
    for (size_t i = n / 2; i > 0; i--) {
        heapify(arr, n, i - 1);
    }
    heapify(arr, n, 0);
    
    // Extract elements from heap one by one
    for (size_t i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

// Security: Input validation function
bool validateInput(const std::vector<int>& arr) {
    // Security: Check array size limit to prevent excessive memory usage
    if (arr.size() > MAX_ARRAY_SIZE) {
        std::cerr << "Error: Array size exceeds maximum allowed size\\n";
        return false;
    }
    return true;
}

int main() {
    // Test case 1: Regular unsorted array
    {
        std::vector<int> arr = {12, 11, 13, 5, 6, 7};
        if (validateInput(arr)) {
            heapSort(arr);
            std::cout << "Test 1: ";
            for (int num : arr) std::cout << num << " ";
            std::cout << "\\n";
        }
    }
    
    // Test case 2: Already sorted array
    {
        std::vector<int> arr = {1, 2, 3, 4, 5};
        if (validateInput(arr)) {
            heapSort(arr);
            std::cout << "Test 2: ";
            for (int num : arr) std::cout << num << " ";
            std::cout << "\\n";
        }
    }
    
    // Test case 3: Reverse sorted array
    {
        std::vector<int> arr = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        if (validateInput(arr)) {
            heapSort(arr);
            std::cout << "Test 3: ";
            for (int num : arr) std::cout << num << " ";
            std::cout << "\\n";
        }
    }
    
    // Test case 4: Array with duplicates
    {
        std::vector<int> arr = {5, 2, 8, 2, 9, 1, 5, 5};
        if (validateInput(arr)) {
            heapSort(arr);
            std::cout << "Test 4: ";
            for (int num : arr) std::cout << num << " ";
            std::cout << "\\n";
        }
    }
    
    // Test case 5: Single element and empty array
    {
        std::vector<int> arr1 = {42};
        if (validateInput(arr1)) {
            heapSort(arr1);
            std::cout << "Test 5a: ";
            for (int num : arr1) std::cout << num << " ";
            std::cout << "\\n";
        }
        
        std::vector<int> arr2 = {};
        if (validateInput(arr2)) {
            heapSort(arr2);
            std::cout << "Test 5b: (empty)\\n";
        }
    }
    
    return 0;
}
