import java.util.*;

public class Task155 {
    public static int[] heapSort(int[] arr) {
        if (arr == null || arr.length <= 1) return arr;
        int n = arr.length;
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }
        for (int i = n - 1; i > 0; i--) {
            int tmp = arr[0];
            arr[0] = arr[i];
            arr[i] = tmp;
            heapify(arr, i, 0);
        }
        return arr;
    }

    private static void heapify(int[] arr, int heapSize, int i) {
        while (true) {
            int largest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < heapSize && arr[left] > arr[largest]) largest = left;
            if (right < heapSize && arr[right] > arr[largest]) largest = right;
            if (largest != i) {
                int t = arr[i];
                arr[i] = arr[largest];
                arr[largest] = t;
                i = largest;
            } else {
                break;
            }
        }
    }

    private static void printArray(int[] a) {
        System.out.println(Arrays.toString(a));
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{4, 10, 3, 5, 1},
            new int[]{},
            new int[]{1},
            new int[]{9, -2, 0, 7, 3, 3, -5},
            new int[]{5, 4, 3, 2, 1}
        };
        for (int[] t : tests) {
            int[] copy = Arrays.copyOf(t, t.length);
            int[] sorted = heapSort(copy);
            printArray(sorted);
        }
    }
}