
import java.util.PriorityQueue;

public class Task147 {
    public static int findKthLargest(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            throw new IllegalArgumentException("Invalid input");
        }
        
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
        // Test case 1
        int[] arr1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test 1: " + findKthLargest(arr1, k1) + " (Expected: 5)");
        
        // Test case 2
        int[] arr2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("Test 2: " + findKthLargest(arr2, k2) + " (Expected: 4)");
        
        // Test case 3
        int[] arr3 = {1};
        int k3 = 1;
        System.out.println("Test 3: " + findKthLargest(arr3, k3) + " (Expected: 1)");
        
        // Test case 4
        int[] arr4 = {7, 10, 4, 3, 20, 15};
        int k4 = 3;
        System.out.println("Test 4: " + findKthLargest(arr4, k4) + " (Expected: 10)");
        
        // Test case 5
        int[] arr5 = {-1, -5, -3, -2, -4};
        int k5 = 2;
        System.out.println("Test 5: " + findKthLargest(arr5, k5) + " (Expected: -2)");
    }
}
