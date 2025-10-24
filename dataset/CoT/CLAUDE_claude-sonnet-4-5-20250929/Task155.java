
import java.util.Arrays;

public class Task155 {
    // Heapify a subtree rooted at index i
    private static void heapify(int[] arr, int n, int i) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // Check if left child exists and is greater than root
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }

        // Check if right child exists and is greater than largest so far
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
        }

        // If largest is not root, swap and continue heapifying
        if (largest != i) {
            int temp = arr[i];
            arr[i] = arr[largest];
            arr[largest] = temp;

            // Recursively heapify the affected sub-tree
            heapify(arr, n, largest);
        }
    }

    // Main heap sort function
    public static int[] heapSort(int[] arr) {
        if (arr == null || arr.length <= 1) {
            return arr;
        }

        // Create a copy to avoid modifying original array
        int[] result = Arrays.copyOf(arr, arr.length);
        int n = result.length;

        // Build max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(result, n, i);
        }

        // Extract elements from heap one by one
        for (int i = n - 1; i > 0; i--) {
            // Move current root to end
            int temp = result[0];
            result[0] = result[i];
            result[i] = temp;

            // Heapify the reduced heap
            heapify(result, i, 0);
        }

        return result;
    }

    public static void main(String[] args) {
        // Test case 1: Random integers
        int[] test1 = {64, 34, 25, 12, 22, 11, 90};
        System.out.println("Test 1 - Input: " + Arrays.toString(test1));
        System.out.println("Test 1 - Output: " + Arrays.toString(heapSort(test1)));

        // Test case 2: Already sorted array
        int[] test2 = {1, 2, 3, 4, 5};
        System.out.println("\\nTest 2 - Input: " + Arrays.toString(test2));
        System.out.println("Test 2 - Output: " + Arrays.toString(heapSort(test2)));

        // Test case 3: Reverse sorted array
        int[] test3 = {9, 7, 5, 3, 1};
        System.out.println("\\nTest 3 - Input: " + Arrays.toString(test3));
        System.out.println("Test 3 - Output: " + Arrays.toString(heapSort(test3)));

        // Test case 4: Array with duplicates
        int[] test4 = {5, 2, 8, 2, 9, 1, 5, 5};
        System.out.println("\\nTest 4 - Input: " + Arrays.toString(test4));
        System.out.println("Test 4 - Output: " + Arrays.toString(heapSort(test4)));

        // Test case 5: Single element array
        int[] test5 = {42};
        System.out.println("\\nTest 5 - Input: " + Arrays.toString(test5));
        System.out.println("Test 5 - Output: " + Arrays.toString(heapSort(test5)));
    }
}
