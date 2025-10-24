import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {

    public int constrainedSubsetSum(int[] nums, int k) {
        int n = nums.length;
        int[] dp = new int[n];
        Deque<Integer> dq = new ArrayDeque<>();
        int maxSum = Integer.MIN_VALUE;

        for (int i = 0; i < n; i++) {
            // Remove indices from the front that are out of the window [i-k, i-1]
            if (!dq.isEmpty() && dq.peekFirst() < i - k) {
                dq.pollFirst();
            }

            // Calculate dp[i]
            // The max sum from the previous k elements is at the front of the deque
            int prevMax = dq.isEmpty() ? 0 : dp[dq.peekFirst()];
            
            // If prevMax is negative, we start a new subsequence from nums[i]
            dp[i] = nums[i] + Math.max(0, prevMax);

            // Maintain the monotonically decreasing property of the deque based on dp values
            while (!dq.isEmpty() && dp[i] >= dp[dq.peekLast()]) {
                dq.pollLast();
            }
            dq.offerLast(i);

            // Update the overall maximum sum
            maxSum = Math.max(maxSum, dp[i]);
        }
        return maxSum;
    }

    public static void main(String[] args) {
        Task177 solution = new Task177();

        // Test Case 1
        int[] nums1 = {10, 2, -10, 5, 20};
        int k1 = 2;
        System.out.println("Test Case 1: " + solution.constrainedSubsetSum(nums1, k1)); // Expected: 37

        // Test Case 2
        int[] nums2 = {-1, -2, -3};
        int k2 = 1;
        System.out.println("Test Case 2: " + solution.constrainedSubsetSum(nums2, k2)); // Expected: -1

        // Test Case 3
        int[] nums3 = {10, -2, -10, -5, 20};
        int k3 = 2;
        System.out.println("Test Case 3: " + solution.constrainedSubsetSum(nums3, k3)); // Expected: 23

        // Test Case 4
        int[] nums4 = {1, -2, 3, 4, -5, 6};
        int k4 = 3;
        System.out.println("Test Case 4: " + solution.constrainedSubsetSum(nums4, k4)); // Expected: 14

        // Test Case 5
        int[] nums5 = {-5, -4, -3, -2, -1};
        int k5 = 5;
        System.out.println("Test Case 5: " + solution.constrainedSubsetSum(nums5, k5)); // Expected: -1
    }
}