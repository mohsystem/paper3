import java.util.ArrayDeque;

public class Task178 {
    // Returns the length of the shortest non-empty subarray with sum at least k, or -1 if none exists.
    public static int shortestSubarray(int[] nums, long k) {
        if (nums == null || nums.length == 0) {
            return -1;
        }
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
            dq.offerLast(i);
        }

        return ans <= n ? ans : -1;
    }

    private static void runTest(int[] nums, long k) {
        int result = shortestSubarray(nums, k);
        System.out.println(result);
    }

    public static void main(String[] args) {
        runTest(new int[]{1}, 1);                 // Expected: 1
        runTest(new int[]{1, 2}, 4);              // Expected: -1
        runTest(new int[]{2, -1, 2}, 3);          // Expected: 3
        runTest(new int[]{84, -37, 32, 40, 95}, 167); // Expected: 3
        runTest(new int[]{17, 85, 93, -45, -21}, 150); // Expected: 2
    }
}