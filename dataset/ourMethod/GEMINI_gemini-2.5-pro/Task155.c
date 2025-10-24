#include <stdio.h>
#include <stdlib.h> // For NULL

// A utility function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * To heapify a subtree rooted at index i.
 * n is the size of the heap.
 */
void heapify(int arr[], int n, int i) {
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
        swap(&arr[i], &arr[largest]);

        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

/**
 * Sorts an array of integers using the Heap Sort algorithm.
 */
void heapSort(int arr[], int n) {
    // Input validation
    if (arr == NULL || n <= 1) {
        return;
    }

    // Build a max-heap from the input data.
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from the heap.
    for (int i = n - 1; i > 0; i--) {
        // Move current root to the end.
        swap(&arr[0], &arr[i]);

        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

// A utility function to print an array
void printArray(const char* label, int arr[], int size) {
    printf("%s", label);
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void runTestCase(const char* testName, int arr[], int n) {
    printf("--- %s ---\n", testName);
    printArray("Original array: ", arr, n);
    heapSort(arr, n);
    printArray("Sorted array:   ", arr, n);
    printf("\n");
}

int main() {
    {
        int arr[] = {12, 11, 13, 5, 6, 7};
        int n = sizeof(arr) / sizeof(arr[0]);
        runTestCase("Test Case 1: General case", arr, n);
    }
    {
        int arr[] = {5, 6, 7, 11, 12, 13};
        int n = sizeof(arr) / sizeof(arr[0]);
        runTestCase("Test Case 2: Already sorted", arr, n);
    }
    {
        int arr[] = {13, 12, 11, 7, 6, 5};
        int n = sizeof(arr) / sizeof(arr[0]);
        runTestCase("Test Case 3: Reverse sorted", arr, n);
    }
    {
        int arr[] = {5, 12, 5, 6, 6, 13};
        int n = sizeof(arr) / sizeof(arr[0]);
        runTestCase("Test Case 4: Array with duplicates", arr, n);
    }
    {
        printf("--- Test Case 5: Empty and single element arrays ---\n");
        int arr_empty[] = {};
        // The size of an empty array is implementation-defined.
        // It's safer to explicitly use 0 for empty arrays.
        int n_empty = 0; 
        printArray("Original array (empty): ", arr_empty, n_empty);
        heapSort(arr_empty, n_empty);
        printArray("Sorted array (empty):   ", arr_empty, n_empty);

        int arr_single[] = {42};
        int n_single = sizeof(arr_single) / sizeof(arr_single[0]);
        printArray("Original array (single): ", arr_single, n_single);
        heapSort(arr_single, n_single);
        printArray("Sorted array (single):   ", arr_single, n_single);
        printf("\n");
    }

    return 0;
}