#include <iostream>
#include <vector>
#include <algorithm> // For std::swap
#include <string>

/**
 * To heapify a subtree rooted at index i.
 * n is the size of the heap.
 */
void heapify(std::vector<int>& arr, int n, int i) {
    int largest = i; // Initialize largest as root
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    // If left child is larger than root
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }

    // If right child is larger than the largest so far
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }

    // If largest is not root
    if (largest != i) {
        std::swap(arr[i], arr[largest]);

        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

/**
 * Sorts a vector of integers using the Heap Sort algorithm.
 */
void heapSort(std::vector<int>& arr) {
    if (arr.size() <= 1) {
        return;
    }
    int n = arr.size();

    // Build a max-heap from the input data.
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from the heap.
    for (int i = n - 1; i > 0; i--) {
        // Move current root to the end.
        std::swap(arr[0], arr[i]);

        // Call max heapify on the reduced heap.
        heapify(arr, i, 0);
    }
}

void printVector(const std::string& label, const std::vector<int>& arr) {
    std::cout << label;
    for (int x : arr) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

void runTestCase(const std::string& testName, std::vector<int> arr) {
    std::cout << "--- " << testName << " ---" << std::endl;
    printVector("Original array: ", arr);
    heapSort(arr);
    printVector("Sorted array:   ", arr);
    std::cout << std::endl;
}

int main() {
    runTestCase("Test Case 1: General case", {12, 11, 13, 5, 6, 7});
    runTestCase("Test Case 2: Already sorted", {5, 6, 7, 11, 12, 13});
    runTestCase("Test Case 3: Reverse sorted", {13, 12, 11, 7, 6, 5});
    runTestCase("Test Case 4: Array with duplicates", {5, 12, 5, 6, 6, 13});
    
    std::cout << "--- Test Case 5: Empty and single element arrays ---" << std::endl;
    runTestCase("Empty array", {});
    runTestCase("Single element array", {42});

    return 0;
}