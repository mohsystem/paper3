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
        int n = nums.length;
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        int minLength = n + 1;
        // Deque stores indices of the prefixSums array.
        Deque<Integer> deque = new ArrayDeque<>();

        for (int j = 0; j <= n; j++) {
            // Condition 1: Find a valid subarray.
            // If prefixSums[j] - prefixSums[deque.peekFirst()] >= k, we found a subarray.
            // We want the shortest, so we check from the left of the deque.
            while (!deque.isEmpty() && prefixSums[j] - prefixSums[deque.peekFirst()] >= k) {
                minLength = Math.min(minLength, j - deque.pollFirst());
            }

            // Condition 2: Maintain a monotonically increasing prefixSums in the deque.
            // If prefixSums[j] <= prefixSums[deque.peekLast()], the last element is no longer optimal.
            // A future subarray starting at index j would be shorter and have a smaller or equal prefix sum.
            while (!deque.isEmpty() && prefixSums[j] <= prefixSums[deque.peekLast()]) {
                deque.pollLast();
            }

            deque.addLast(j);
        }

        return minLength == n + 1 ? -1 : minLength;
    }

    public static void main(String[] args) {
        Task178 solution = new Task178();

        // Test case 1: Basic case
        int[] nums1 = {1};
        int k1 = 1;
        System.out.println("Test 1: " + solution.shortestSubarray(nums1, k1)); // Expected: 1

        // Test case 2: No such subarray
        int[] nums2 = {1, 2};
        int k2 = 4;
        System.out.println("Test 2: " + solution.shortestSubarray(nums2, k2)); // Expected: -1

        // Test case 3: With negative numbers
        int[] nums3 = {2, -1, 2};
        int k3 = 3;
        System.out.println("Test 3: " + solution.shortestSubarray(nums3, k3)); // Expected: 3

        // Test case 4: More complex case
        int[] nums4 = {84, -37, 32, 40, 95};
        int k4 = 167;
        System.out.println("Test 4: " + solution.shortestSubarray(nums4, k4)); // Expected: 3

        // Test case 5: All negative numbers
        int[] nums5 = {-1, -2, -3};
        int k5 = 1;
        System.out.println("Test 5: " + solution.shortestSubarray(nums5, k5)); // Expected: -1
    }
}