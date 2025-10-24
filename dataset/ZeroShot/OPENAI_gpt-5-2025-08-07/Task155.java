import java.util.Arrays;

public class Task155 {
    public static int[] heapSort(int[] input) {
        if (input == null) {
            return new int[0];
        }
        int n = input.length;
        int[] a = Arrays.copyOf(input, n);

        // Build max heap
        for (int i = (n >>> 1) - 1; i >= 0; i--) {
            siftDown(a, n, i);
        }

        // Extract elements from heap
        for (int end = n - 1; end > 0; end--) {
            swap(a, 0, end);
            siftDown(a, end, 0);
        }
        return a;
    }

    private static void siftDown(int[] a, int n, int i) {
        while (true) {
            int largest = i;
            int left = (i << 1) + 1;
            int right = left + 1;

            if (left < n && a[left] > a[largest]) largest = left;
            if (right < n && a[right] > a[largest]) largest = right;

            if (largest == i) break;
            swap(a, i, largest);
            i = largest;
        }
    }

    private static void swap(int[] a, int i, int j) {
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {},
            {1},
            {5, 3, 8, 4, 2, 7, 1, 10},
            {1, 2, 3, 4, 5},
            {-3, -1, -7, 0, 2, 2, -3}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] sorted = heapSort(tests[i]);
            printArray(sorted);
        }
    }
}