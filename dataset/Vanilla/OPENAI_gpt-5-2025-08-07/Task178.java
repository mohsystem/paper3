import java.util.*;

public class Task178 {
    public static int shortestSubarray(int[] nums, int k) {
        int n = nums.length;
        long[] ps = new long[n + 1];
        for (int i = 0; i < n; i++) ps[i + 1] = ps[i] + nums[i];

        Deque<Integer> dq = new ArrayDeque<>();
        int ans = n + 1;

        for (int i = 0; i <= n; i++) {
            while (!dq.isEmpty() && ps[i] - ps[dq.peekFirst()] >= k) {
                ans = Math.min(ans, i - dq.pollFirst());
            }
            while (!dq.isEmpty() && ps[i] <= ps[dq.peekLast()]) {
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
            {17, 85, 93, -45, -21},
            {48, 99, 37, 4, -31}
        };
        int[] ks = {1, 4, 3, 150, 140};
        for (int i = 0; i < tests.length; i++) {
            System.out.println(shortestSubarray(tests[i], ks[i]));
        }
    }
}