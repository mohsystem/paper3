
import java.util.PriorityQueue;

public class Task147 {
    public static int findKthLargest(int[] nums, int k) {
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
        int[] nums1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test 1: " + findKthLargest(nums1, k1)); // Expected: 5
        
        // Test case 2
        int[] nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("Test 2: " + findKthLargest(nums2, k2)); // Expected: 4
        
        // Test case 3
        int[] nums3 = {7, 10, 4, 3, 20, 15};
        int k3 = 3;
        System.out.println("Test 3: " + findKthLargest(nums3, k3)); // Expected: 10
        
        // Test case 4
        int[] nums4 = {1};
        int k4 = 1;
        System.out.println("Test 4: " + findKthLargest(nums4, k4)); // Expected: 1
        
        // Test case 5
        int[] nums5 = {-1, -5, -3, -7, -2};
        int k5 = 2;
        System.out.println("Test 5: " + findKthLargest(nums5, k5)); // Expected: -2
    }
}
