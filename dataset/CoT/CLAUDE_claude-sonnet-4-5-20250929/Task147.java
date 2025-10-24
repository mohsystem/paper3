
import java.util.Arrays;
import java.util.PriorityQueue;

public class Task147 {
    // Method to find kth largest element using min heap
    public static Integer findKthLargest(int[] nums, int k) {
        // Input validation
        if (nums == null || nums.length == 0) {
            throw new IllegalArgumentException("Array cannot be null or empty");
        }
        if (k <= 0 || k > nums.length) {
            throw new IllegalArgumentException("k must be between 1 and array length");
        }
        
        // Use min heap of size k to maintain k largest elements
        PriorityQueue<Integer> minHeap = new PriorityQueue<>();
        
        for (int num : nums) {
            minHeap.offer(num);
            if (minHeap.size() > k) {
                minHeap.poll();
            }
        }
        
        return minHeap.peek();
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal case
        int[] test1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test 1: Array: " + Arrays.toString(test1) + ", k=" + k1);
        System.out.println("Result: " + findKthLargest(test1, k1));
        System.out.println();
        
        // Test case 2: Array with duplicates
        int[] test2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("Test 2: Array: " + Arrays.toString(test2) + ", k=" + k2);
        System.out.println("Result: " + findKthLargest(test2, k2));
        System.out.println();
        
        // Test case 3: k = 1 (largest element)
        int[] test3 = {7, 10, 4, 3, 20, 15};
        int k3 = 1;
        System.out.println("Test 3: Array: " + Arrays.toString(test3) + ", k=" + k3);
        System.out.println("Result: " + findKthLargest(test3, k3));
        System.out.println();
        
        // Test case 4: k equals array length (smallest element)
        int[] test4 = {5, 2, 8, 1, 9};
        int k4 = 5;
        System.out.println("Test 4: Array: " + Arrays.toString(test4) + ", k=" + k4);
        System.out.println("Result: " + findKthLargest(test4, k4));
        System.out.println();
        
        // Test case 5: Single element array
        int[] test5 = {42};
        int k5 = 1;
        System.out.println("Test 5: Array: " + Arrays.toString(test5) + ", k=" + k5);
        System.out.println("Result: " + findKthLargest(test5, k5));
    }
}
