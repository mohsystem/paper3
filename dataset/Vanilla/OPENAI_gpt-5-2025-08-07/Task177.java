import java.util.*;

public class Task177 {
    public static int constrainedSubsetSum(int[] nums, int k) {
        int n = nums.length;
        int[] dp = new int[n];
        Deque<Integer> dq = new ArrayDeque<>();
        int ans = Integer.MIN_VALUE;

        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && i - dq.peekFirst() > k) dq.pollFirst();
            int bestPrev = dq.isEmpty() ? 0 : Math.max(0, dp[dq.peekFirst()]);
            dp[i] = nums[i] + bestPrev;
            ans = Math.max(ans, dp[i]);
            if (dp[i] > 0) {
                while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) dq.pollLast();
                dq.offerLast(i);
            }
        }
        return ans;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {10, 2, -10, 5, 20},
            {-1, -2, -3},
            {10, -2, -10, -5, 20},
            {1, -1, -1, -1, 5},
            {5, -1, -2, 10}
        };
        int[] ks = {2, 1, 2, 2, 3};

        for (int t = 0; t < tests.length; t++) {
            int res = constrainedSubsetSum(tests[t], ks[t]);
            System.out.println(res);
        }
    }
}