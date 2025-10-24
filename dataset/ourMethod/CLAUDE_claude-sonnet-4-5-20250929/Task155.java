
import java.util.Arrays;
import java.util.Scanner;

public class Task155 {
    private static final int MAX_ARRAY_SIZE = 1000000;
    
    public static int[] heapSort(int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        if (arr.length > MAX_ARRAY_SIZE) {
            throw new IllegalArgumentException("Array size exceeds maximum allowed size");
        }
        
        int[] result = Arrays.copyOf(arr, arr.length);
        int n = result.length;
        
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(result, n, i);
        }
        
        for (int i = n - 1; i > 0; i--) {
            int temp = result[0];
            result[0] = result[i];
            result[i] = temp;
            heapify(result, i, 0);
        }
        
        return result;
    }
    
    private static void heapify(int[] arr, int n, int i) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }
        
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
        }
        
        if (largest != i) {
            int swap = arr[i];
            arr[i] = arr[largest];
            arr[largest] = swap;
            heapify(arr, n, largest);
        }
    }
    
    public static void main(String[] args) {
        int[][] testCases = {
            {64, 34, 25, 12, 22, 11, 90},
            {5, 2, 8, 1, 9},
            {-5, -2, -8, -1, -9},
            {1},
            {100, 50, 75, 25, 30, 60, 80}
        };
        
        for (int i = 0; i < testCases.length; i++) {
            try {
                int[] original = testCases[i];
                int[] sorted = heapSort(original);
                System.out.println("Test case " + (i + 1) + ":");
                System.out.println("Original: " + Arrays.toString(original));
                System.out.println("Sorted:   " + Arrays.toString(sorted));
                System.out.println();
            } catch (IllegalArgumentException e) {
                System.err.println("Error in test case " + (i + 1) + ": " + e.getMessage());
            }
        }
    }
}
