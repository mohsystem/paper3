import java.util.ArrayDeque;
import java.util.Deque;

public class Task177 {
    public static long constrainedSubsetSum(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0) return 0L;
        int n = nums.length;
        long[] dp = new long[n];
        Deque<Integer> dq = new ArrayDeque<>();
        long ans = Long.MIN_VALUE;
        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && dq.peekFirst() < i - k) dq.pollFirst();
            long best = dq.isEmpty() ? 0L : dp[dq.peekFirst()];
            dp[i] = (long) nums[i] + Math.max(0L, best);
            ans = Math.max(ans, dp[i]);
            while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) dq.pollLast();
            dq.offerLast(i);
        }
        return ans;
    }

    private static void runTest(int[] nums, int k) {
        long res = constrainedSubsetSum(nums, k);
        System.out.println(res);
    }

    public static void main(String[] args) {
        runTest(new int[]{10, 2, -10, 5, 20}, 2);
        runTest(new int[]{-1, -2, -3}, 1);
        runTest(new int[]{10, -2, -10, -5, 20}, 2);
        runTest(new int[]{1, -1, -2, 4, -7, 3}, 2);
        runTest(new int[]{5, -1, 5}, 1);
    }
}