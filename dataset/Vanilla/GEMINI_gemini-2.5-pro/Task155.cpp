#include <iostream>
#include <vector>
#include <algorithm> // for std::swap

// To heapify a subtree rooted with node i which is an index in arr[]
// n is the size of the heap
void heapify(std::vector<int>& arr, int n, int i) {
    int largest = i; // Initialize largest as root
    int leftChild = 2 * i + 1;
    int rightChild = 2 * i + 2;

    // If left child is larger than root
    if (leftChild < n && arr[leftChild] > arr[largest]) {
        largest = leftChild;
    }

    // If right child is larger than largest so far
    if (rightChild < n && arr[rightChild] > arr[largest]) {
        largest = rightChild;
    }

    // If largest is not root
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

// main function to do heap sort
void heapSort(std::vector<int>& arr) {
    int n = arr.size();

    // Build max-heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        std::swap(arr[0], arr[i]);
        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

// A utility function to print a vector
void printVector(const std::string& label, const std::vector<int>& arr) {
    std::cout << label;
    for (int i : arr) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {12, 11, 13, 5, 6, 7};
    std::cout << "Test Case 1:" << std::endl;
    printVector("Original array: ", arr1);
    heapSort(arr1);
    printVector("Sorted array:   ", arr1);
    std::cout << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {5, 4, 3, 2, 1};
    std::cout << "Test Case 2:" << std::endl;
    printVector("Original array: ", arr2);
    heapSort(arr2);
    printVector("Sorted array:   ", arr2);
    std::cout << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {1, 2, 3, 4, 5};
    std::cout << "Test Case 3:" << std::endl;
    printVector("Original array: ", arr3);
    heapSort(arr3);
    printVector("Sorted array:   ", arr3);
    std::cout << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {10, -5, 8, 0, -1, 3};
    std::cout << "Test Case 4:" << std::endl;
    printVector("Original array: ", arr4);
    heapSort(arr4);
    printVector("Sorted array:   ", arr4);
    std::cout << std::endl;

    // Test Case 5
    std::vector<int> arr5 = {42};
    std::cout << "Test Case 5:" << std::endl;
    printVector("Original array: ", arr5);
    heapSort(arr5);
    printVector("Sorted array:   ", arr5);
    std::cout << std::endl;

    return 0;
}