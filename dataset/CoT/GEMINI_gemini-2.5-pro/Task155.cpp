#include <iostream>
#include <vector>
#include <algorithm> // For std::swap

/**
 * To heapify a subtree rooted with node i which is an index in arr.
 * n is the size of the heap.
 * @param arr The vector representing the heap.
 * @param n The size of the heap.
 * @param i The index of the root of the subtree to heapify.
 */
void heapify(std::vector<int>& arr, int n, int i) {
    int largest = i; // Initialize largest as root
    int l = 2 * i + 1; // left = 2*i + 1
    int r = 2 * i + 2; // right = 2*i + 2

    // If left child is larger than root
    if (l < n && arr[l] > arr[largest]) {
        largest = l;
    }

    // If right child is larger than largest so far
    if (r < n && arr[r] > arr[largest]) {
        largest = r;
    }

    // If largest is not root
    if (largest != i) {
        std::swap(arr[i], arr[largest]);

        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

/**
 * The main function to sort a vector using Heap Sort.
 * @param arr The vector to be sorted.
 */
void heapSort(std::vector<int>& arr) {
    if (arr.empty()) {
        return;
    }
    int n = arr.size();

    // Build a max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from the heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        std::swap(arr[0], arr[i]);

        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

void printVector(const std::vector<int>& arr) {
    for (int val : arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<std::vector<int>> testCases = {
        {12, 11, 13, 5, 6, 7},
        {5, 4, 3, 2, 1},
        {1, 2, 3, 4, 5},
        {-5, 8, -2, 0, 9, 1, -10},
        {42}
    };
    
    for (int i = 0; i < testCases.size(); ++i) {
        std::vector<int> arr = testCases[i];
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Original array: ";
        printVector(arr);
        heapSort(arr);
        std::cout << "Sorted array: ";
        printVector(arr);
        std::cout << std::endl;
    }

    return 0;
}