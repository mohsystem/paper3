import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Deque;

public class Task178 {

    // Returns length of the shortest non-empty subarray with sum at least k, or -1 if none exists.
    public static int shortestSubarray(int[] nums, long k) {
        if (nums == null || nums.length == 0) return -1;
        final int n = nums.length;
        if (n < 1 || n > 1_000_000) return -1; // basic validation per security rules

        long[] prefix = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = prefix[i] + nums[i];
        }

        int ans = n + 1;
        Deque<Integer> dq = new ArrayDeque<>(n + 1);

        for (int i = 0; i <= n; i++) {
            while (!dq.isEmpty() && prefix[i] - prefix[dq.peekFirst()] >= k) {
                int idx = dq.pollFirst();
                int length = i - idx;
                if (length < ans) ans = length;
            }
            while (!dq.isEmpty() && prefix[i] <= prefix[dq.peekLast()]) {
                dq.pollLast();
            }
            dq.addLast(i);
        }

        return ans <= n ? ans : -1;
    }

    // Simple pretty printer for arrays
    private static String arrToString(int[] arr) {
        return Arrays.toString(arr);
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1},
            {1, 2},
            {2, -1, 2},
            {84, -37, 32, 40, 95},
            {17, 85, 93, -45, -21}
        };
        long[] ks = new long[]{1, 4, 3, 167, 150};

        for (int i = 0; i < tests.length; i++) {
            int res = shortestSubarray(tests[i], ks[i]);
            System.out.println("Test " + (i + 1) + " nums=" + arrToString(tests[i]) + " k=" + ks[i] + " -> " + res);
        }
    }
}