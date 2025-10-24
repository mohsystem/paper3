
import java.util.ArrayDeque;
import java.util.Deque;

public class Task178 {
    public static int shortestSubarray(int[] nums, int k) {
        int n = nums.length;
        long[] prefixSum = new long[n + 1];
        
        // Calculate prefix sums
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        int minLength = Integer.MAX_VALUE;
        Deque<Integer> deque = new ArrayDeque<>();
        
        for (int i = 0; i <= n; i++) {
            // Remove indices from front where sum >= k
            while (!deque.isEmpty() && prefixSum[i] - prefixSum[deque.peekFirst()] >= k) {
                minLength = Math.min(minLength, i - deque.pollFirst());
            }
            
            // Maintain increasing order of prefix sums in deque
            while (!deque.isEmpty() && prefixSum[i] <= prefixSum[deque.peekLast()]) {
                deque.pollLast();
            }
            
            deque.addLast(i);
        }
        
        return minLength == Integer.MAX_VALUE ? -1 : minLength;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {1};
        int k1 = 1;
        System.out.println("Test 1: " + shortestSubarray(nums1, k1)); // Expected: 1
        
        // Test case 2
        int[] nums2 = {1, 2};
        int k2 = 4;
        System.out.println("Test 2: " + shortestSubarray(nums2, k2)); // Expected: -1
        
        // Test case 3
        int[] nums3 = {2, -1, 2};
        int k3 = 3;
        System.out.println("Test 3: " + shortestSubarray(nums3, k3)); // Expected: 3
        
        // Test case 4
        int[] nums4 = {17, 85, 93, -45, -21};
        int k4 = 150;
        System.out.println("Test 4: " + shortestSubarray(nums4, k4)); // Expected: 2
        
        // Test case 5
        int[] nums5 = {-28, 81, -20, 28, -29};
        int k5 = 89;
        System.out.println("Test 5: " + shortestSubarray(nums5, k5)); // Expected: 3
    }
}
