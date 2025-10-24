import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {

    public static int constrainedSubsetSum(int[] nums, int k) {
        int n = nums.length;
        // dp[i] will store the maximum sum of a subsequence ending at index i
        int[] dp = new int[n];
        // A deque to store indices of dp elements, maintaining them in decreasing order of their values
        Deque<Integer> dq = new ArrayDeque<>();
        int maxSum = Integer.MIN_VALUE;

        for (int i = 0; i < n; i++) {
            // Remove indices from the front of the deque that are out of the k-sized window
            while (!dq.isEmpty() && dq.peekFirst() < i - k) {
                dq.pollFirst();
            }

            // The max sum of a subsequence ending in the previous window is at the front of the deque
            int prevMax = dq.isEmpty() ? 0 : dp[dq.peekFirst()];
            
            // Calculate dp[i]: it's nums[i] plus the max sum from the previous valid subsequence.
            // If the previous max sum is negative, we start a new subsequence from nums[i].
            dp[i] = nums[i] + Math.max(0, prevMax);

            // Maintain the deque in decreasing order of dp values
            // Remove elements from the back of the deque that are smaller than the current dp[i]
            while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) {
                dq.pollLast();
            }
            
            // Add the current index to the deque
            dq.offerLast(i);

            // Update the overall maximum sum found so far
            maxSum = Math.max(maxSum, dp[i]);
        }

        return maxSum;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] nums1 = {10, 2, -10, 5, 20};
        int k1 = 2;
        System.out.println("Test Case 1: " + constrainedSubsetSum(nums1, k1)); // Expected: 37

        // Test Case 2
        int[] nums2 = {-1, -2, -3};
        int k2 = 1;
        System.out.println("Test Case 2: " + constrainedSubsetSum(nums2, k2)); // Expected: -1

        // Test Case 3
        int[] nums3 = {10, -2, -10, -5, 20};
        int k3 = 2;
        System.out.println("Test Case 3: " + constrainedSubsetSum(nums3, k3)); // Expected: 23

        // Test Case 4
        int[] nums4 = {-5, -4, -3, -2, -1};
        int k4 = 3;
        System.out.println("Test Case 4: " + constrainedSubsetSum(nums4, k4)); // Expected: -1
        
        // Test Case 5
        int[] nums5 = {1, -5, 2, -6, 3, -7, 4};
        int k5 = 2;
        System.out.println("Test Case 5: " + constrainedSubsetSum(nums5, k5)); // Expected: 10
    }
}