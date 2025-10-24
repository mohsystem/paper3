import java.util.Arrays;

public class Task155 {

    // Main function to perform heap sort
    public void heapSort(int arr[]) {
        int n = arr.length;

        // Build a max-heap from the array
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // One by one extract an element from the heap
        for (int i = n - 1; i > 0; i--) {
            // Move the current root (maximum element) to the end
            int temp = arr[0];
            arr[0] = arr[i];
            arr[i] = temp;

            // Call max heapify on the reduced heap
            heapify(arr, i, 0);
        }
    }

    // To heapify a subtree rooted with node i which is an index in arr[].
    // n is the size of the heap
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

        // If the largest is not the root
        if (largest != i) {
            int swap = arr[i];
            arr[i] = arr[largest];
            arr[largest] = swap;

            // Recursively heapify the affected sub-tree
            heapify(arr, n, largest);
        }
    }

    public static void main(String args[]) {
        Task155 sorter = new Task155();

        // Test Case 1
        int[] arr1 = {12, 11, 13, 5, 6, 7};
        System.out.println("Test Case 1:");
        System.out.println("Original array: " + Arrays.toString(arr1));
        sorter.heapSort(arr1);
        System.out.println("Sorted array:   " + Arrays.toString(arr1));
        System.out.println();

        // Test Case 2
        int[] arr2 = {5, 4, 3, 2, 1};
        System.out.println("Test Case 2:");
        System.out.println("Original array: " + Arrays.toString(arr2));
        sorter.heapSort(arr2);
        System.out.println("Sorted array:   " + Arrays.toString(arr2));
        System.out.println();

        // Test Case 3
        int[] arr3 = {1, 2, 3, 4, 5};
        System.out.println("Test Case 3:");
        System.out.println("Original array: " + Arrays.toString(arr3));
        sorter.heapSort(arr3);
        System.out.println("Sorted array:   " + Arrays.toString(arr3));
        System.out.println();

        // Test Case 4
        int[] arr4 = {10, -5, 8, 0, -1, 3};
        System.out.println("Test Case 4:");
        System.out.println("Original array: " + Arrays.toString(arr4));
        sorter.heapSort(arr4);
        System.out.println("Sorted array:   " + Arrays.toString(arr4));
        System.out.println();

        // Test Case 5
        int[] arr5 = {42};
        System.out.println("Test Case 5:");
        System.out.println("Original array: " + Arrays.toString(arr5));
        sorter.heapSort(arr5);
        System.out.println("Sorted array:   " + Arrays.toString(arr5));
        System.out.println();
    }
}