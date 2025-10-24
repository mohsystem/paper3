import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Deque;

public class Task177 {

    public static long constrainedSubsequenceSum(int[] nums, int k) {
        if (nums == null) {
            throw new IllegalArgumentException("nums must not be null");
        }
        int n = nums.length;
        if (n == 0) {
            throw new IllegalArgumentException("nums must be non-empty");
        }
        if (k < 1 || k > n) {
            throw new IllegalArgumentException("k must satisfy 1 <= k <= nums.length");
        }

        long[] dp = new long[n];
        long ans = Long.MIN_VALUE;
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && dq.peekFirst() < i - k) {
                dq.removeFirst();
            }
            long bestPrev = 0L;
            if (!dq.isEmpty()) {
                bestPrev = Math.max(0L, dp[dq.peekFirst()]);
            }
            dp[i] = (long) nums[i] + bestPrev;
            ans = Math.max(ans, dp[i]);

            while (!dq.isEmpty() && dp[dq.peekLast()] <= dp[i]) {
                dq.removeLast();
            }
            dq.addLast(i);
        }
        return ans;
    }

    private static void runTest(int[] nums, int k) {
        long res = constrainedSubsequenceSum(nums, k);
        System.out.println("Result: " + res);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(new int[]{10, 2, -10, 5, 20}, 2);      // Expected 37
        runTest(new int[]{-1, -2, -3}, 1);             // Expected -1
        runTest(new int[]{10, -2, -10, -5, 20}, 2);    // Expected 23
        runTest(new int[]{5}, 1);                      // Expected 5
        runTest(new int[]{1, -1, 1, -1, 1}, 4);        // Expected 3
    }
}