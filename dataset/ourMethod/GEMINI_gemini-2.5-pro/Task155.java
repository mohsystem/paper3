import java.util.Arrays;

public class Task155 {

    /**
     * Sorts an array of integers using the Heap Sort algorithm.
     * @param arr The array to be sorted.
     */
    public void heapSort(int[] arr) {
        if (arr == null || arr.length <= 1) {
            return;
        }
        int n = arr.length;

        // Build a max-heap from the input data.
        // The last parent node is at index (n/2 - 1).
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // One by one extract elements from the heap.
        for (int i = n - 1; i > 0; i--) {
            // Move the current root (maximum element) to the end.
            int temp = arr[0];
            arr[0] = arr[i];
            arr[i] = temp;

            // Call max heapify on the reduced heap.
            heapify(arr, i, 0);
        }
    }

    /**
     * To heapify a subtree rooted at index i.
     * n is the size of the heap.
     */
    private void heapify(int[] arr, int n, int i) {
        int largest = i; // Initialize largest as root
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // If the left child is larger than the root
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }

        // If the right child is larger than the largest so far
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
        }

        // If the largest element is not the root
        if (largest != i) {
            int swap = arr[i];
            arr[i] = arr[largest];
            arr[largest] = swap;

            // Recursively heapify the affected sub-tree
            heapify(arr, n, largest);
        }
    }

    public static void main(String[] args) {
        Task155 sorter = new Task155();

        System.out.println("--- Test Case 1: General case ---");
        int[] arr1 = {12, 11, 13, 5, 6, 7};
        System.out.println("Original array: " + Arrays.toString(arr1));
        sorter.heapSort(arr1);
        System.out.println("Sorted array:   " + Arrays.toString(arr1));
        System.out.println();

        System.out.println("--- Test Case 2: Already sorted ---");
        int[] arr2 = {5, 6, 7, 11, 12, 13};
        System.out.println("Original array: " + Arrays.toString(arr2));
        sorter.heapSort(arr2);
        System.out.println("Sorted array:   " + Arrays.toString(arr2));
        System.out.println();

        System.out.println("--- Test Case 3: Reverse sorted ---");
        int[] arr3 = {13, 12, 11, 7, 6, 5};
        System.out.println("Original array: " + Arrays.toString(arr3));
        sorter.heapSort(arr3);
        System.out.println("Sorted array:   " + Arrays.toString(arr3));
        System.out.println();
        
        System.out.println("--- Test Case 4: Array with duplicates ---");
        int[] arr4 = {5, 12, 5, 6, 6, 13};
        System.out.println("Original array: " + Arrays.toString(arr4));
        sorter.heapSort(arr4);
        System.out.println("Sorted array:   " + Arrays.toString(arr4));
        System.out.println();
        
        System.out.println("--- Test Case 5: Empty and single element arrays ---");
        int[] arr5_empty = {};
        System.out.println("Original array (empty): " + Arrays.toString(arr5_empty));
        sorter.heapSort(arr5_empty);
        System.out.println("Sorted array (empty):   " + Arrays.toString(arr5_empty));
        
        int[] arr5_single = {42};
        System.out.println("Original array (single): " + Arrays.toString(arr5_single));
        sorter.heapSort(arr5_single);
        System.out.println("Sorted array (single):   " + Arrays.toString(arr5_single));
        System.out.println();
    }
}