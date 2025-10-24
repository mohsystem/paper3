#include <iostream>
#include <vector>
#include <utility> // For std::swap
#include <string>

class Task155 {
private:
    /**
     * Helper function to heapify a subtree rooted with node i.
     *
     * @param arr The vector representing the heap.
     * @param n   The size of the heap.
     * @param i   The index of the root of the subtree to heapify.
     */
    void heapify(std::vector<int>& arr, int n, int i) {
        int largest = i; // Initialize largest as root
        int leftChild = 2 * i + 1;
        int rightChild = 2 * i + 2;

        // If the left child is larger than the root
        if (leftChild < n && arr[leftChild] > arr[largest]) {
            largest = leftChild;
        }

        // If the right child is larger than the largest so far
        if (rightChild < n && arr[rightChild] > arr[largest]) {
            largest = rightChild;
        }

        // If the largest element is not the root, swap them.
        if (largest != i) {
            std::swap(arr[i], arr[largest]);

            // Recursively heapify the affected sub-tree.
            heapify(arr, n, largest);
        }
    }

public:
    /**
     * Sorts a vector of integers using the Heap Sort algorithm.
     * The method sorts the vector in-place.
     *
     * @param arr The vector of integers to be sorted.
     */
    void heapSort(std::vector<int>& arr) {
        // A secure code should handle trivial inputs gracefully.
        if (arr.size() <= 1) {
            return;
        }

        int n = arr.size();

        // Build a max-heap from the input data.
        // We start from the last non-leaf node and move up to the root.
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // One by one, extract elements from the heap.
        for (int i = n - 1; i > 0; i--) {
            // Move the current root (maximum value) to the end of the vector.
            std::swap(arr[0], arr[i]);

            // Call heapify on the reduced heap to restore the max-heap property.
            // The size of the heap is now 'i'.
            heapify(arr, i, 0);
        }
    }
};

void printVector(const std::string& label, const std::vector<int>& arr) {
    std::cout << label;
    for (int val : arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    Task155 sorter;

    // Test Cases
    std::vector<std::vector<int>> testCases = {
        {12, 11, 13, 5, 6, 7},      // Test Case 1: Random elements
        {},                             // Test Case 2: Empty vector
        {5},                            // Test Case 3: Single element
        {1, 2, 3, 4, 5, 6},         // Test Case 4: Already sorted
        {6, 5, 4, 3, 2, 1}          // Test Case 5: Reverse sorted
    };

    int testNum = 1;
    for (auto& arr : testCases) {
        std::cout << "Test Case " << testNum << std::endl;
        printVector("Original array: ", arr);
        sorter.heapSort(arr);
        printVector("Sorted array:   ", arr);
        std::cout << "--------------------" << std::endl;
        testNum++;
    }

    return 0;
}