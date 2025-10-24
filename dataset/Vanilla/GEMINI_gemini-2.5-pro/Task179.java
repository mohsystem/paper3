import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Deque;

public class Task179 {

    /**
     * Finds the maximum value in a sliding window of size k.
     *
     * @param nums The input array of integers.
     * @param k    The size of the sliding window.
     * @return An array containing the maximum of each sliding window.
     */
    public int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            return new int[0];
        }

        int n = nums.length;
        int[] result = new int[n - k + 1];
        int resultIndex = 0;
        
        // Deque stores indices of elements in the current window.
        // The indices point to values in decreasing order.
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i < n; i++) {
            // Remove indices from the front of the deque that are out of the current window.
            if (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }

            // Maintain the decreasing order of values in the deque.
            // Remove indices from the back whose corresponding values are smaller than the current element.
            while (!dq.isEmpty() && nums[dq.peekLast()] < nums[i]) {
                dq.pollLast();
            }

            // Add the current index to the back of the deque.
            dq.offerLast(i);

            // Once the window is full (i.e., we have processed at least k elements),
            // the maximum for the current window is at the front of the deque.
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
        System.out.println();

        // Test Case 2
        int[] nums2 = {1};
        int k2 = 1;
        System.out.println("Test Case 2:");
        System.out.println("Input: nums = " + Arrays.toString(nums2) + ", k = " + k2);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums2, k2)));
        System.out.println();

        // Test Case 3
        int[] nums3 = {1, -1};
        int k3 = 1;
        System.out.println("Test Case 3:");
        System.out.println("Input: nums = " + Arrays.toString(nums3) + ", k = " + k3);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums3, k3)));
        System.out.println();

        // Test Case 4
        int[] nums4 = {7, 2, 4};
        int k4 = 2;
        System.out.println("Test Case 4:");
        System.out.println("Input: nums = " + Arrays.toString(nums4) + ", k = " + k4);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums4, k4)));
        System.out.println();

        // Test Case 5
        int[] nums5 = {1, 3, 1, 2, 0, 5};
        int k5 = 3;
        System.out.println("Test Case 5:");
        System.out.println("Input: nums = " + Arrays.toString(nums5) + ", k = " + k5);
        System.out.println("Output: " + Arrays.toString(solution.maxSlidingWindow(nums5, k5)));
        System.out.println();
    }
}