import java.util.Arrays;

public class Task155 {
    private static final int MAX_LEN = 1_000_000;

    public static int[] heapSort(int[] input) {
        if (input == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        if (input.length < 0 || input.length > MAX_LEN) {
            throw new IllegalArgumentException("Invalid input length.");
        }
        int[] arr = Arrays.copyOf(input, input.length);
        int n = arr.length;

        // Build max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            siftDown(arr, i, n);
        }

        // Extract elements from heap
        for (int end = n - 1; end > 0; end--) {
            swap(arr, 0, end);
            siftDown(arr, 0, end);
        }
        return arr;
    }

    private static void siftDown(int[] arr, int start, int heapSize) {
        int root = start;
        while (true) {
            int left = 2 * root + 1;
            if (left >= heapSize) break;
            int right = left + 1;
            int largest = root;
            if (arr[left] > arr[largest]) largest = left;
            if (right < heapSize && arr[right] > arr[largest]) largest = right;
            if (largest == root) break;
            swap(arr, root, largest);
            root = largest;
        }
    }

    private static void swap(int[] a, int i, int j) {
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        try {
            int[][] tests = new int[][]{
                {3, 1, 4, 1, 5, 9, 2, 6, 5},
                {},
                {42},
                {1, 2, 3, 4, 5},
                {0, -1, -3, 2, -1, 2, 0}
            };

            for (int i = 0; i < tests.length; i++) {
                int[] sorted = heapSort(tests[i]);
                printArray(sorted);
            }
        } catch (IllegalArgumentException ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }
}