#include <stdio.h>
#include <stdlib.h> // For NULL

/**
 * Helper function to swap two integers.
 * @param a Pointer to the first integer.
 * @param b Pointer to the second integer.
 */
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Helper function to heapify a subtree rooted with node i.
 *
 * @param arr The array representing the heap.
 * @param n   The size of the heap.
 * @param i   The index of the root of the subtree to heapify.
 */
void heapify(int arr[], int n, int i) {
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
        swap(&arr[i], &arr[largest]);

        // Recursively heapify the affected sub-tree.
        heapify(arr, n, largest);
    }
}

/**
 * Sorts an array of integers using the Heap Sort algorithm.
 * The function sorts the array in-place.
 *
 * @param arr The array of integers to be sorted.
 * @param n   The number of elements in the array.
 */
void heapSort(int arr[], int n) {
    // A secure code should handle NULL or trivial inputs gracefully.
    if (arr == NULL || n <= 1) {
        return;
    }

    // Build a max-heap from the input data.
    // We start from the last non-leaf node and move up to the root.
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one, extract elements from the heap.
    for (int i = n - 1; i > 0; i--) {
        // Move the current root (maximum value) to the end of the array.
        swap(&arr[0], &arr[i]);

        // Call heapify on the reduced heap to restore the max-heap property.
        // The size of the heap is now 'i'.
        heapify(arr, i, 0);
    }
}

void printArray(const char* label, int arr[], int size) {
    printf("%s", label);
    for (int i = 0; i < size; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1: Random elements
    int arr1[] = {12, 11, 13, 5, 6, 7};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1\n");
    printArray("Original array: ", arr1, n1);
    heapSort(arr1, n1);
    printArray("Sorted array:   ", arr1, n1);
    printf("--------------------\n");

    // Test Case 2: Empty array
    int arr2[] = {};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2\n");
    printArray("Original array: ", arr2, n2);
    heapSort(arr2, n2);
    printArray("Sorted array:   ", arr2, n2);
    printf("--------------------\n");
    
    // Test Case 3: Single element
    int arr3[] = {5};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3\n");
    printArray("Original array: ", arr3, n3);
    heapSort(arr3, n3);
    printArray("Sorted array:   ", arr3, n3);
    printf("--------------------\n");

    // Test Case 4: Already sorted
    int arr4[] = {1, 2, 3, 4, 5, 6};
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4\n");
    printArray("Original array: ", arr4, n4);
    heapSort(arr4, n4);
    printArray("Sorted array:   ", arr4, n4);
    printf("--------------------\n");
    
    // Test Case 5: Reverse sorted
    int arr5[] = {6, 5, 4, 3, 2, 1};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5\n");
    printArray("Original array: ", arr5, n5);
    heapSort(arr5, n5);
    printArray("Sorted array:   ", arr5, n5);
    printf("--------------------\n");

    return 0;
}