import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {

    /**
     * Finds the maximum sum of a non-empty subsequence of the array `nums`
     * such that for every two consecutive integers in the subsequence, nums[i] and nums[j],
     * where i < j, the condition j - i <= k is satisfied.
     *
     * @param nums The input integer array.
     * @param k The maximum distance between indices of consecutive elements in the subsequence.
     * @return The maximum subsequence sum.
     */
    public int constrainedSubsetSum(int[] nums, int k) {
        if (nums == null || nums.length == 0) {
            return 0;
        }

        int n = nums.length;
        // dp[i] will store the maximum subsequence sum ending at index i.
        int[] dp = new int[n];
        // Deque will store indices `j` such that dp[j] are in decreasing order.
        Deque<Integer> dq = new ArrayDeque<>();
        int maxSum = Integer.MIN_VALUE;

        for (int i = 0; i < n; i++) {
            // 1. Remove indices from the front of the deque that are out of the window [i-k, i-1].
            if (!dq.isEmpty() && dq.peekFirst() < i - k) {
                dq.pollFirst();
            }

            // 2. Calculate dp[i]. The max sum of a valid subsequence ending before i
            //    is at the front of the deque.
            int maxPrev = 0;
            if (!dq.isEmpty()) {
                maxPrev = dp[dq.peekFirst()];
            }
            
            // If maxPrev is negative, we start a new subsequence from nums[i].
            dp[i] = nums[i] + Math.max(0, maxPrev);

            // 3. Maintain the decreasing property of dp values in the deque.
            //    If dp[i] is greater than dp values at the end of the deque,
            //    those smaller values can never be the maximum in any future window.
            while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) {
                dq.pollLast();
            }
            dq.offerLast(i);

            // 4. Update the overall maximum sum found so far.
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
        int[] nums4 = {-5, -6};
        int k4 = 2;
        System.out.println("Test Case 4: " + solution.constrainedSubsetSum(nums4, k4)); // Expected: -5
        
        // Test Case 5
        int[] nums5 = {1, -5, -6, 2, 8, -4, 3};
        int k5 = 3;
        System.out.println("Test Case 5: " + solution.constrainedSubsetSum(nums5, k5)); // Expected: 14
    }
}