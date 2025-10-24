import java.util.ArrayDeque;
import java.util.Deque;

public class Task178 {

    /**
     * Finds the length of the shortest non-empty subarray with a sum of at least k.
     *
     * @param nums The input integer array.
     * @param k The target sum.
     * @return The length of the shortest subarray, or -1 if no such subarray exists.
     */
    public int shortestSubarray(int[] nums, int k) {
        if (nums == null || nums.length == 0) {
            return -1;
        }

        int n = nums.length;
        // Use long for prefix sums to avoid integer overflow
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        int minLength = Integer.MAX_VALUE;
        // Deque stores indices of the prefixSums array
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i <= n; i++) {
            // Condition 1: Find a valid subarray
            // If P[i] - P[dq.front()] >= k, we have a candidate subarray.
            while (!dq.isEmpty() && prefixSums[i] - prefixSums[dq.peekFirst()] >= k) {
                minLength = Math.min(minLength, i - dq.pollFirst());
            }

            // Condition 2: Maintain monotonic property (increasing prefix sums)
            // If P[i] <= P[dq.back()], dq.back() is a worse starting point than i.
            while (!dq.isEmpty() && prefixSums[i] <= prefixSums[dq.peekLast()]) {
                dq.pollLast();
            }

            dq.offerLast(i);
        }

        return minLength == Integer.MAX_VALUE ? -1 : minLength;
    }

    public static void main(String[] args) {
        Task178 solution = new Task178();

        // Test Case 1
        int[] nums1 = {1};
        int k1 = 1;
        System.out.println("Test Case 1: " + solution.shortestSubarray(nums1, k1));

        // Test Case 2
        int[] nums2 = {1, 2};
        int k2 = 4;
        System.out.println("Test Case 2: " + solution.shortestSubarray(nums2, k2));

        // Test Case 3
        int[] nums3 = {2, -1, 2};
        int k3 = 3;
        System.out.println("Test Case 3: " + solution.shortestSubarray(nums3, k3));

        // Test Case 4
        int[] nums4 = {84, -37, 32, 40, 95};
        int k4 = 167;
        System.out.println("Test Case 4: " + solution.shortestSubarray(nums4, k4));

        // Test Case 5
        int[] nums5 = {-28, 81, -20, 28, -29};
        int k5 = 89;
        System.out.println("Test Case 5: " + solution.shortestSubarray(nums5, k5));
    }
}