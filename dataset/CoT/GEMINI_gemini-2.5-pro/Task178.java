import java.util.ArrayDeque;
import java.util.Deque;

public class Task178 {

    public int shortestSubarray(int[] nums, int k) {
        int n = nums.length;
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        int minLength = n + 1;
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i <= n; i++) {
            // Check if any subarray ending at i-1 satisfies the condition
            while (!dq.isEmpty() && prefixSums[i] - prefixSums[dq.peekFirst()] >= k) {
                minLength = Math.min(minLength, i - dq.pollFirst());
            }
            // Maintain the monotonic (increasing) property of the deque
            while (!dq.isEmpty() && prefixSums[i] <= prefixSums[dq.peekLast()]) {
                dq.pollLast();
            }
            dq.offerLast(i);
        }

        return minLength > n ? -1 : minLength;
    }

    public static void main(String[] args) {
        Task178 solver = new Task178();
        // Test Case 1
        int[] nums1 = {1};
        int k1 = 1;
        System.out.println("Test Case 1: " + solver.shortestSubarray(nums1, k1));

        // Test Case 2
        int[] nums2 = {1, 2};
        int k2 = 4;
        System.out.println("Test Case 2: " + solver.shortestSubarray(nums2, k2));

        // Test Case 3
        int[] nums3 = {2, -1, 2};
        int k3 = 3;
        System.out.println("Test Case 3: " + solver.shortestSubarray(nums3, k3));

        // Test Case 4
        int[] nums4 = {84, -37, 32, 40, 95};
        int k4 = 167;
        System.out.println("Test Case 4: " + solver.shortestSubarray(nums4, k4));

        // Test Case 5
        int[] nums5 = {-28, 81, -20, 28, -29};
        int k5 = 89;
        System.out.println("Test Case 5: " + solver.shortestSubarray(nums5, k5));
    }
}