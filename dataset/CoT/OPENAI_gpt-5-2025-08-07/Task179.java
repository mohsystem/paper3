import java.util.*;

public class Task179 {
    // Step 1: Problem understanding and functionality
    // Step 2-4: Security and code review embedded via safe checks and robust handling
    public static int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            return new int[0];
        }
        int n = nums.length;
        int[] result = new int[n - k + 1];
        Deque<Integer> dq = new ArrayDeque<>(); // stores indices

        for (int i = 0; i < n; i++) {
            // Remove indices out of the current window
            while (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }
            // Maintain decreasing deque
            while (!dq.isEmpty() && nums[dq.peekLast()] <= nums[i]) {
                dq.pollLast();
            }
            dq.offerLast(i);
            // Record result
            if (i >= k - 1) {
                result[i - k + 1] = nums[dq.peekFirst()];
            }
        }
        return result;
    }

    private static void printArray(int[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    // Step 5: Secure code output with tests
    public static void main(String[] args) {
        int[][] tests = {
            {1,3,-1,-3,5,3,6,7},
            {1},
            {9,10,9,-7,-4,-8,2,-6},
            {1,-1},
            {7,2,4}
        };
        int[] ks = {3,1,5,1,2};

        for (int t = 0; t < tests.length; t++) {
            int[] res = maxSlidingWindow(tests[t], ks[t]);
            printArray(res);
        }
    }
}