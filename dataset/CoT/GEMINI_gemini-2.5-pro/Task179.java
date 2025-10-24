import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Arrays;

public class Task179 {

    /**
     * Finds the maximum value in a sliding window of size k.
     *
     * @param nums The input array of integers.
     * @param k The size of the sliding window.
     * @return An array containing the maximum of each sliding window.
     */
    public static int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            return new int[0];
        }

        int n = nums.length;
        int[] result = new int[n - k + 1];
        int resultIndex = 0;

        // Deque stores indices of elements in the current window.
        // The elements corresponding to these indices are in decreasing order.
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i < n; i++) {
            // Remove indices from the front of the deque that are out of the current window.
            if (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }

            // Maintain the decreasing order property of the deque.
            // Remove indices from the back whose corresponding elements are smaller than or equal to the current element.
            while (!dq.isEmpty() && nums[dq.peekLast()] <= nums[i]) {
                dq.pollLast();
            }

            // Add the current element's index to the back of the deque.
            dq.offerLast(i);

            // Once the window is fully formed, the maximum element is at the front of the deque.
            if (i >= k - 1) {
                result[resultIndex++] = nums[dq.peekFirst()];
            }
        }

        return result;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
        int k1 = 3;
        System.out.println("Test Case 1: nums = " + Arrays.toString(nums1) + ", k = " + k1);
        System.out.println("Output: " + Arrays.toString(maxSlidingWindow(nums1, k1))); // Expected: [3, 3, 5, 5, 6, 7]

        // Test Case 2
        int[] nums2 = {1};
        int k2 = 1;
        System.out.println("\nTest Case 2: nums = " + Arrays.toString(nums2) + ", k = " + k2);
        System.out.println("Output: " + Arrays.toString(maxSlidingWindow(nums2, k2))); // Expected: [1]
        
        // Test Case 3
        int[] nums3 = {1, -1};
        int k3 = 1;
        System.out.println("\nTest Case 3: nums = " + Arrays.toString(nums3) + ", k = " + k3);
        System.out.println("Output: " + Arrays.toString(maxSlidingWindow(nums3, k3))); // Expected: [1, -1]

        // Test Case 4
        int[] nums4 = {9, 9, 9, 9};
        int k4 = 2;
        System.out.println("\nTest Case 4: nums = " + Arrays.toString(nums4) + ", k = " + k4);
        System.out.println("Output: " + Arrays.toString(maxSlidingWindow(nums4, k4))); // Expected: [9, 9, 9]

        // Test Case 5
        int[] nums5 = {7, 6, 5, 4, 3, 2, 1};
        int k5 = 4;
        System.out.println("\nTest Case 5: nums = " + Arrays.toString(nums5) + ", k = " + k5);
        System.out.println("Output: " + Arrays.toString(maxSlidingWindow(nums5, k5))); // Expected: [7, 6, 5, 4]
    }
}