import java.util.ArrayDeque;

public class Task178 {
    public static int shortestSubarray(int[] nums, long k) {
        int n = nums.length;
        long[] prefix = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = prefix[i] + nums[i];
        }
        ArrayDeque<Integer> dq = new ArrayDeque<>();
        int ans = n + 1;
        for (int i = 0; i <= n; i++) {
            while (!dq.isEmpty() && prefix[i] - prefix[dq.peekFirst()] >= k) {
                ans = Math.min(ans, i - dq.pollFirst());
            }
            while (!dq.isEmpty() && prefix[i] <= prefix[dq.peekLast()]) {
                dq.pollLast();
            }
            dq.addLast(i);
        }
        return ans <= n ? ans : -1;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {1},
            {1, 2},
            {2, -1, 2},
            {84, -37, 32, 40, 95},
            {17, 85, 93, -45, -21}
        };
        long[] ks = {1, 4, 3, 167, 150};

        for (int i = 0; i < tests.length; i++) {
            System.out.println(shortestSubarray(tests[i], ks[i]));
        }
    }
}