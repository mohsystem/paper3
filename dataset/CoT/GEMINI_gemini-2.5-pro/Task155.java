import java.util.Arrays;

public class Task155 {

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
            int swap = arr[i];
            arr[i] = arr[largest];
            arr[largest] = swap;

            // Recursively heapify the affected sub-tree
            heapify(arr, n, largest);
        }
    }

    /**
     * The main function to sort an array of given size using Heap Sort.
     * @param arr The array to be sorted.
     */
    public void heapSort(int arr[]) {
        if (arr == null || arr.length == 0) {
            return;
        }
        int n = arr.length;

        // Build a max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // One by one extract an element from the heap
        for (int i = n - 1; i > 0; i--) {
            // Move current root to end
            int temp = arr[0];
            arr[0] = arr[i];
            arr[i] = temp;

            // call max heapify on the reduced heap
            heapify(arr, i, 0);
        }
    }

    public static void main(String[] args) {
        Task155 sorter = new Task155();

        int[][] testCases = {
            {12, 11, 13, 5, 6, 7},
            {5, 4, 3, 2, 1},
            {1, 2, 3, 4, 5},
            {-5, 8, -2, 0, 9, 1, -10},
            {42}
        };

        for (int i = 0; i < testCases.length; i++) {
            int[] arr = testCases[i];
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original array: " + Arrays.toString(arr));
            sorter.heapSort(arr);
            System.out.println("Sorted array: " + Arrays.toString(arr));
            System.out.println();
        }
    }
}