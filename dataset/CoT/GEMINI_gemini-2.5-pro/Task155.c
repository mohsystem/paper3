#include <stdio.h>

// A utility function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * To heapify a subtree rooted with node i which is an index in arr[].
 * n is the size of the heap.
 * @param arr The array representing the heap.
 * @param n The size of the heap.
 * @param i The index of the root of the subtree to heapify.
 */
void heapify(int arr[], int n, int i) {
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
        swap(&arr[i], &arr[largest]);

        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

/**
 * The main function to sort an array of given size using Heap Sort.
 * @param arr The array to be sorted.
 * @param n The size of the array.
 */
void heapSort(int arr[], int n) {
    if (arr == NULL || n <= 0) {
        return;
    }

    // Build a max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // One by one extract an element from the heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(&arr[0], &arr[i]);

        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

// A utility function to print an array
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1
    int arr1[] = {12, 11, 13, 5, 6, 7};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1:\n");
    printf("Original array: ");
    printArray(arr1, n1);
    heapSort(arr1, n1);
    printf("Sorted array: ");
    printArray(arr1, n1);
    printf("\n");

    // Test Case 2
    int arr2[] = {5, 4, 3, 2, 1};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2:\n");
    printf("Original array: ");
    printArray(arr2, n2);
    heapSort(arr2, n2);
    printf("Sorted array: ");
    printArray(arr2, n2);
    printf("\n");

    // Test Case 3
    int arr3[] = {1, 2, 3, 4, 5};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3:\n");
    printf("Original array: ");
    printArray(arr3, n3);
    heapSort(arr3, n3);
    printf("Sorted array: ");
    printArray(arr3, n3);
    printf("\n");

    // Test Case 4
    int arr4[] = {-5, 8, -2, 0, 9, 1, -10};
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4:\n");
    printf("Original array: ");
    printArray(arr4, n4);
    heapSort(arr4, n4);
    printf("Sorted array: ");
    printArray(arr4, n4);
    printf("\n");
    
    // Test Case 5
    int arr5[] = {42};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5:\n");
    printf("Original array: ");
    printArray(arr5, n5);
    heapSort(arr5, n5);
    printf("Sorted array: ");
    printArray(arr5, n5);
    printf("\n");
    
    return 0;
}