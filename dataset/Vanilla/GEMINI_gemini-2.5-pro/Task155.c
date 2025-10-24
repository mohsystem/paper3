#include <stdio.h>

// A utility function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// To heapify a subtree rooted with node i which is an index in arr[].
// n is the size of the heap
void heapify(int arr[], int n, int i) {
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
        swap(&arr[i], &arr[largest]);
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

// main function to do heap sort
void heapSort(int arr[], int n) {
    // Build max-heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(&arr[0], &arr[i]);
        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

// A utility function to print an array of size n
void printArray(const char* label, int arr[], int n) {
    printf("%s", label);
    for (int i = 0; i < n; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1
    int arr1[] = {12, 11, 13, 5, 6, 7};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1:\n");
    printArray("Original array: ", arr1, n1);
    heapSort(arr1, n1);
    printArray("Sorted array:   ", arr1, n1);
    printf("\n");

    // Test Case 2
    int arr2[] = {5, 4, 3, 2, 1};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2:\n");
    printArray("Original array: ", arr2, n2);
    heapSort(arr2, n2);
    printArray("Sorted array:   ", arr2, n2);
    printf("\n");

    // Test Case 3
    int arr3[] = {1, 2, 3, 4, 5};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3:\n");
    printArray("Original array: ", arr3, n3);
    heapSort(arr3, n3);
    printArray("Sorted array:   ", arr3, n3);
    printf("\n");

    // Test Case 4
    int arr4[] = {10, -5, 8, 0, -1, 3};
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4:\n");
    printArray("Original array: ", arr4, n4);
    heapSort(arr4, n4);
    printArray("Sorted array:   ", arr4, n4);
    printf("\n");

    // Test Case 5
    int arr5[] = {42};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5:\n");
    printArray("Original array: ", arr5, n5);
    heapSort(arr5, n5);
    printArray("Sorted array:   ", arr5, n5);
    printf("\n");

    return 0;
}