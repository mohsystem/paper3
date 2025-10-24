
import java.util.PriorityQueue;
import java.util.Arrays;

public class Task147 {
    /**
     * Finds the kth largest element in an array using a min-heap.
     * @param arr The input array of integers
     * @param k The position of the largest element to find (1-indexed)
     * @return The kth largest element
     * @throws IllegalArgumentException if inputs are invalid
     */
    public static int findKthLargest(int[] arr, int k) {
        // Input validation
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        if (arr.length == 0) {
            throw new IllegalArgumentException("Array cannot be empty");
        }
        if (k < 1 || k > arr.length) {
            throw new IllegalArgumentException("k must be between 1 and array length");
        }
        
        // Use a min-heap of size k to track the k largest elements
        PriorityQueue<Integer> minHeap = new PriorityQueue<>(k);
        
        for (int num : arr) {
            minHeap.offer(num);
            if (minHeap.size() > k) {
                minHeap.poll();
            }
        }
        
        // The root of the min-heap is the kth largest element
        Integer result = minHeap.peek();
        if (result == null) {
            throw new IllegalStateException("Unexpected null result");
        }
        return result;
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Normal case
            int[] arr1 = {3, 2, 1, 5, 6, 4};
            int k1 = 2;
            System.out.println("Test 1: arr=" + Arrays.toString(arr1) + ", k=" + k1 + 
                             " => " + findKthLargest(arr1, k1));
            
            // Test case 2: k=1 (largest element)
            int[] arr2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
            int k2 = 1;
            System.out.println("Test 2: arr=" + Arrays.toString(arr2) + ", k=" + k2 + 
                             " => " + findKthLargest(arr2, k2));
            
            // Test case 3: k equals array length (smallest element)
            int[] arr3 = {7, 10, 4, 3, 20, 15};
            int k3 = 6;
            System.out.println("Test 3: arr=" + Arrays.toString(arr3) + ", k=" + k3 + 
                             " => " + findKthLargest(arr3, k3));
            
            // Test case 4: Array with duplicates
            int[] arr4 = {1, 1, 1, 1, 1};
            int k4 = 3;
            System.out.println("Test 4: arr=" + Arrays.toString(arr4) + ", k=" + k4 + 
                             " => " + findKthLargest(arr4, k4));
            
            // Test case 5: Single element array
            int[] arr5 = {42};
            int k5 = 1;
            System.out.println("Test 5: arr=" + Arrays.toString(arr5) + ", k=" + k5 + 
                             " => " + findKthLargest(arr5, k5));
            
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
