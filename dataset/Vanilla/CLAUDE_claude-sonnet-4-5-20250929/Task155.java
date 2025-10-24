
import java.util.Arrays;

public class Task155 {
    public static void heapSort(int[] arr) {
        int n = arr.length;
        
        // Build max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }
        
        // Extract elements from heap one by one
        for (int i = n - 1; i > 0; i--) {
            // Move current root to end
            int temp = arr[0];
            arr[0] = arr[i];
            arr[i] = temp;
            
            // Heapify the reduced heap
            heapify(arr, i, 0);
        }
    }
    
    private static void heapify(int[] arr, int n, int i) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        // If left child is larger than root
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }
        
        // If right child is larger than largest so far
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
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
    
    public static void main(String[] args) {
        // Test case 1
        int[] arr1 = {12, 11, 13, 5, 6, 7};
        System.out.println("Test 1 - Original: " + Arrays.toString(arr1));
        heapSort(arr1);
        System.out.println("Test 1 - Sorted: " + Arrays.toString(arr1));
        
        // Test case 2
        int[] arr2 = {64, 34, 25, 12, 22, 11, 90};
        System.out.println("\\nTest 2 - Original: " + Arrays.toString(arr2));
        heapSort(arr2);
        System.out.println("Test 2 - Sorted: " + Arrays.toString(arr2));
        
        // Test case 3
        int[] arr3 = {5, 4, 3, 2, 1};
        System.out.println("\\nTest 3 - Original: " + Arrays.toString(arr3));
        heapSort(arr3);
        System.out.println("Test 3 - Sorted: " + Arrays.toString(arr3));
        
        // Test case 4
        int[] arr4 = {1};
        System.out.println("\\nTest 4 - Original: " + Arrays.toString(arr4));
        heapSort(arr4);
        System.out.println("Test 4 - Sorted: " + Arrays.toString(arr4));
        
        // Test case 5
        int[] arr5 = {-5, 10, -3, 0, 8, -20, 15};
        System.out.println("\\nTest 5 - Original: " + Arrays.toString(arr5));
        heapSort(arr5);
        System.out.println("Test 5 - Sorted: " + Arrays.toString(arr5));
    }
}
