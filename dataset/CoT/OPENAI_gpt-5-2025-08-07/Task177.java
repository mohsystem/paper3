import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {
    // Computes the maximum sum of a non-empty subsequence with constraint j - i <= k
    public static long constrainedSubsetSum(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0) {
            throw new IllegalArgumentException("Invalid input");
        }
        int n = nums.length;
        long[] dp = new long[n];
        long ans = Long.MIN_VALUE;

        Deque<Integer> dq = new ArrayDeque<>(); // store indices with decreasing dp values

        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && dq.peekFirst() < i - k) {
                dq.pollFirst();
            }
            long bestPrev = 0;
            if (!dq.isEmpty() && dp[dq.peekFirst()] > 0) {
                bestPrev = dp[dq.peekFirst()];
            }
            dp[i] = (long) nums[i] + bestPrev;
            ans = Math.max(ans, dp[i]);

            while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) {
                dq.pollLast();
            }
            dq.offerLast(i);
        }
        return ans;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {10, 2, -10, 5, 20},
            {-1, -2, -3},
            {10, -2, -10, -5, 20},
            {1, -1, -2, 4, -7, 3},
            {-5}
        };
        int[] ks = {2, 1, 2, 2, 1};
        long[] expected = {37, -1, 23, 7, -5};

        for (int t = 0; t < tests.length; t++) {
            long res = constrainedSubsetSum(tests[t], ks[t]);
            System.out.println("Result " + (t + 1) + ": " + res + " (Expected: " + expected[t] + ")");
        }
    }
}