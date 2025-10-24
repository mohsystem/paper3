import java.util.Arrays;

public class Task155 {

    /**
     * Sorts an array of integers using the Heap Sort algorithm.
     * The method sorts the array in-place.
     *
     * @param arr The array of integers to be sorted.
     */
    public void heapSort(int[] arr) {
        // A secure code should handle null or trivial inputs gracefully.
        if (arr == null || arr.length <= 1) {
            return;
        }

        int n = arr.length;

        // Build a max-heap from the input data.
        // We start from the last non-leaf node and move up to the root.
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // One by one, extract elements from the heap.
        for (int i = n - 1; i > 0; i--) {
            // Move the current root (maximum value) to the end of the array.
            int temp = arr[0];
            arr[0] = arr[i];
            arr[i] = temp;

            // Call heapify on the reduced heap to restore the max-heap property.
            // The size of the heap is now 'i'.
            heapify(arr, i, 0);
        }
    }

    /**
     * Helper function to heapify a subtree rooted with node i.
     *
     * @param arr The array representing the heap.
     * @param n   The size of the heap.
     * @param i   The index of the root of the subtree to heapify.
     */
    private void heapify(int[] arr, int n, int i) {
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
            int swap = arr[i];
            arr[i] = arr[largest];
            arr[largest] = swap;

            // Recursively heapify the affected sub-tree.
            heapify(arr, n, largest);
        }
    }

    public static void main(String[] args) {
        Task155 sorter = new Task155();

        // Test Cases
        int[][] testCases = {
            {12, 11, 13, 5, 6, 7},      // Test Case 1: Random elements
            {},                             // Test Case 2: Empty array
            {5},                            // Test Case 3: Single element
            {1, 2, 3, 4, 5, 6},         // Test Case 4: Already sorted
            {6, 5, 4, 3, 2, 1}          // Test Case 5: Reverse sorted
        };

        for (int i = 0; i < testCases.length; i++) {
            int[] arr = testCases[i].clone(); // Clone to preserve original for comparison if needed
            System.out.println("Test Case " + (i + 1));
            System.out.println("Original array: " + Arrays.toString(arr));
            sorter.heapSort(arr);
            System.out.println("Sorted array:   " + Arrays.toString(arr));
            System.out.println("--------------------");
        }
    }
}