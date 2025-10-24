// Chain-of-Through process:
// 1) Problem understanding: Implement heap sort that takes an int array and returns a sorted array.
// 2) Security requirements: Avoid modifying inputs, handle nulls/empties safely, and ensure index bounds.
// 3) Secure coding generation: Copy inputs, use safe loops, and avoid integer overflows in indexing logic.
// 4) Code review: Checked for null checks, array bounds, and safe swaps.
// 5) Secure code output: Final version with thorough validations and test cases.

import java.util.Arrays;

public class Task155 {

    public static int[] heapSort(int[] input) {
        if (input == null) {
            return null;
        }
        int n = input.length;
        int[] arr = Arrays.copyOf(input, n); // Do not mutate input

        // Build max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // Extract elements one by one
        for (int i = n - 1; i > 0; i--) {
            swap(arr, 0, i);
            heapify(arr, i, 0);
        }
        return arr;
    }

    private static void heapify(int[] arr, int heapSize, int i) {
        // Iterative heapify to avoid deep recursion
        while (true) {
            int largest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < heapSize && arr[left] > arr[largest]) {
                largest = left;
            }
            if (right < heapSize && arr[right] > arr[largest]) {
                largest = right;
            }
            if (largest != i) {
                swap(arr, i, largest);
                i = largest;
            } else {
                break;
            }
        }
    }

    private static void swap(int[] arr, int i, int j) {
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }

    private static void printArray(int[] arr) {
        if (arr == null) {
            System.out.println("null");
        } else {
            System.out.println(Arrays.toString(arr));
        }
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {4, 10, 3, 5, 1},
            {},
            {1},
            {5, -1, 3, -2, 0, 5, 5},
            {1000000000, -1000000000, Integer.MAX_VALUE, Integer.MIN_VALUE, 42}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] input = tests[i];
            int[] sorted = heapSort(input);
            System.out.print("Test " + (i + 1) + " input:  ");
            printArray(input);
            System.out.print("Test " + (i + 1) + " sorted: ");
            printArray(sorted);
            System.out.println();
        }
    }
}