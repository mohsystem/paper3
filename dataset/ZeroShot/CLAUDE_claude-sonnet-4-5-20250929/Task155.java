
import java.util.Arrays;

public class Task155 {
    public static void heapSort(int[] arr) {
        if (arr == null || arr.length <= 1) {
            return;
        }
        
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
        // Test case 1: Regular unsorted array
        int[] arr1 = {12, 11, 13, 5, 6, 7};
        heapSort(arr1);
        System.out.println("Test 1: " + Arrays.toString(arr1));
        
        // Test case 2: Already sorted array
        int[] arr2 = {1, 2, 3, 4, 5};
        heapSort(arr2);
        System.out.println("Test 2: " + Arrays.toString(arr2));
        
        // Test case 3: Reverse sorted array
        int[] arr3 = {9, 7, 5, 3, 1};
        heapSort(arr3);
        System.out.println("Test 3: " + Arrays.toString(arr3));
        
        // Test case 4: Array with duplicates
        int[] arr4 = {5, 2, 8, 2, 9, 1, 5};
        heapSort(arr4);
        System.out.println("Test 4: " + Arrays.toString(arr4));
        
        // Test case 5: Single element array
        int[] arr5 = {42};
        heapSort(arr5);
        System.out.println("Test 5: " + Arrays.toString(arr5));
    }
}
