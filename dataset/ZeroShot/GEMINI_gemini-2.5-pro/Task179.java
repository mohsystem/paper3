import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Arrays;

public class Task179 {

    public int[] maxSlidingWindow(int[] nums, int k) {
        // Handle edge cases based on constraints
        if (nums == null || nums.length == 0 || k <= 0) {
            return new int[0];
        }
        
        int n = nums.length;
        // The number of windows and the size of the result array
        int[] result = new int[n - k + 1];
        int resultIndex = 0;

        // Deque stores indices of elements in nums. The values corresponding
        // to these indices are in decreasing order.
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i < n; i++) {
            // 1. Remove indices from the front that are out of the current window's scope
            if (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }

            // 2. Maintain the decreasing order of values in the deque.
            // Remove indices from the back whose corresponding values are smaller than the current element's value.
            while (!dq.isEmpty() && nums[dq.peekLast()] < nums[i]) {
                dq.pollLast();
            }

            // 3. Add the current element's index to the deque
            dq.offerLast(i);

            // 4. If the window has at least k elements, the front of the deque is the max.
            // Add it to the result array.
            if (i >= k - 1) {
                result[resultIndex++] = nums[dq.peekFirst()];
            }
        }

        return result;
    }

    public static void main(String[] args) {
        Task179 solution = new Task179();

        // Test Case 1
        int[] nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
        int k1 = 3;
        System.out.println("Test Case 1:");
        System.out.println("Input: nums = " + Arrays.toString(nums1) + ", k = " + k1);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums1, k1)));
        System.out.println("Expected: [3, 3, 5, 5, 6, 7]\n");

        // Test Case 2
        int[] nums2 = {1};
        int k2 = 1;
        System.out.println("Test Case 2:");
        System.out.println("Input: nums = " + Arrays.toString(nums2) + ", k = " + k2);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums2, k2)));
        System.out.println("Expected: [1]\n");

        // Test Case 3
        int[] nums3 = {1, -1};
        int k3 = 1;
        System.out.println("Test Case 3:");
        System.out.println("Input: nums = " + Arrays.toString(nums3) + ", k = " + k3);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums3, k3)));
        System.out.println("Expected: [1, -1]\n");

        // Test Case 4
        int[] nums4 = {9, 10, 9, -7, -4, -8, 2, -6};
        int k4 = 5;
        System.out.println("Test Case 4:");
        System.out.println("Input: nums = " + Arrays.toString(nums4) + ", k = " + k4);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums4, k4)));
        System.out.println("Expected: [10, 10, 9, 2, 2]\n");

        // Test Case 5
        int[] nums5 = {7, 2, 4};
        int k5 = 2;
        System.out.println("Test Case 5:");
        System.out.println("Input: nums = " + Arrays.toString(nums5) + ", k = " + k5);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums5, k5)));
        System.out.println("Expected: [7, 4]\n");
    }
}