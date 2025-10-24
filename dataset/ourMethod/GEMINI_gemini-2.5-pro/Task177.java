import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {

    /**
     * Calculates the maximum sum of a non-empty subsequence of the array
     * such that for every two consecutive integers in the subsequence,
     * nums[i] and nums[j] with i < j, the condition j - i <= k is satisfied.
     *
     * @param nums The input integer array.
     * @param k The constraint for the distance between consecutive indices.
     * @return The maximum constrained subsequence sum.
     */
    public int constrainedSubsetSum(int[] nums, int k) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        int n = nums.length;
        // dp[i] will store the max sum of a subsequence ending at index i
        int[] dp = new int[n];
        // Deque to store indices for the sliding window maximum
        Deque<Integer> deque = new ArrayDeque<>();
        int maxSum = Integer.MIN_VALUE;

        for (int i = 0; i < n; i++) {
            // Remove indices from the front that are out of the window [i-k, i-1]
            if (!deque.isEmpty() && deque.peekFirst() < i - k) {
                deque.pollFirst();
            }

            // Calculate dp[i]. The max sum from the previous valid window is at the front of the deque.
            int prevMax = 0;
            if (!deque.isEmpty()) {
                prevMax = dp[deque.peekFirst()];
            }
            
            // If prevMax is positive, we extend the subsequence. Otherwise, we start a new one with nums[i].
            dp[i] = nums[i] + Math.max(0, prevMax);

            // Maintain the decreasing property of the deque (in terms of dp values)
            while (!deque.isEmpty() && dp[deque.peekLast()] <= dp[i]) {
                deque.pollLast();
            }
            deque.offerLast(i);

            // Update the overall maximum sum found so far
            maxSum = Math.max(maxSum, dp[i]);
        }
        return maxSum;
    }

    public static void main(String[] args) {
        Task177 solver = new Task177();
        
        // Test Case 1
        int[] nums1 = {10, 2, -10, 5, 20};
        int k1 = 2;
        System.out.println("Test Case 1: " + solver.constrainedSubsetSum(nums1, k1));

        // Test Case 2
        int[] nums2 = {-1, -2, -3};
        int k2 = 1;
        System.out.println("Test Case 2: " + solver.constrainedSubsetSum(nums2, k2));

        // Test Case 3
        int[] nums3 = {10, -2, -10, -5, 20};
        int k3 = 2;
        System.out.println("Test Case 3: " + solver.constrainedSubsetSum(nums3, k3));

        // Test Case 4
        int[] nums4 = {-8269, 3217, -4023, -4138, -683, 6455, -3621, 9242, 4015, -3790};
        int k4 = 1;
        System.out.println("Test Case 4: " + solver.constrainedSubsetSum(nums4, k4));
        
        // Test Case 5
        int[] nums5 = {1, -1, -2, -3, 5};
        int k5 = 2;
        System.out.println("Test Case 5: " + solver.constrainedSubsetSum(nums5, k5));
    }
}