
#include <stdio.h>

void heapify(int arr[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    // If left child is larger than root
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }
    
    // If right child is larger than largest so far
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }
    
    // If largest is not root
    if (largest != i) {
        int temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;
        
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

void heapSort(int arr[], int n) {
    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }
    
    // Extract elements from heap one by one
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        int temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;
        
        // Heapify the reduced heap
        heapify(arr, i, 0);
    }
}

void printArray(int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    int arr1[] = {12, 11, 13, 5, 6, 7};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test 1 - Original: ");
    printArray(arr1, n1);
    heapSort(arr1, n1);
    printf("Test 1 - Sorted: ");
    printArray(arr1, n1);
    
    // Test case 2
    int arr2[] = {64, 34, 25, 12, 22, 11, 90};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("\\nTest 2 - Original: ");
    printArray(arr2, n2);
    heapSort(arr2, n2);
    printf("Test 2 - Sorted: ");
    printArray(arr2, n2);
    
    // Test case 3
    int arr3[] = {5, 4, 3, 2, 1};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("\\nTest 3 - Original: ");
    printArray(arr3, n3);
    heapSort(arr3, n3);
    printf("Test 3 - Sorted: ");
    printArray(arr3, n3);
    
    // Test case 4
    int arr4[] = {1};
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("\\nTest 4 - Original: ");
    printArray(arr4, n4);
    heapSort(arr4, n4);
    printf("Test 4 - Sorted: ");
    printArray(arr4, n4);
    
    // Test case 5
    int arr5[] = {-5, 10, -3, 0, 8, -20, 15};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("\\nTest 5 - Original: ");
    printArray(arr5, n5);
    heapSort(arr5, n5);
    printf("Test 5 - Sorted: ");
    printArray(arr5, n5);
    
    return 0;
}
