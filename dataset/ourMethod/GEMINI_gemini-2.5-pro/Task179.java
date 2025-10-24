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
    public static int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            return new int[0];
        }

        int n = nums.length;
        int[] result = new int[n - k + 1];
        int resultIndex = 0;
        
        // Deque stores indices of elements from nums.
        Deque<Integer> dq = new ArrayDeque<>();

        for (int i = 0; i < n; i++) {
            // 1. Remove indices from the front that are out of the current window.
            if (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }

            // 2. Remove indices from the back whose corresponding elements are smaller than the current element.
            // This maintains a deque of indices whose elements are in decreasing order.
            while (!dq.isEmpty() && nums[dq.peekLast()] < nums[i]) {
                dq.pollLast();
            }

            // 3. Add the current index to the back.
            dq.offerLast(i);

            // 4. Once the window is full (i.e., we have at least k elements),
            // the maximum for the current window is at the front of the deque.
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
        int[] output1 = maxSlidingWindow(nums1, k1);
        System.out.println("Test Case 1: " + Arrays.toString(output1)); // Expected: [3, 3, 5, 5, 6, 7]

        // Test Case 2
        int[] nums2 = {1};
        int k2 = 1;
        int[] output2 = maxSlidingWindow(nums2, k2);
        System.out.println("Test Case 2: " + Arrays.toString(output2)); // Expected: [1]

        // Test Case 3
        int[] nums3 = {1, -1};
        int k3 = 1;
        int[] output3 = maxSlidingWindow(nums3, k3);
        System.out.println("Test Case 3: " + Arrays.toString(output3)); // Expected: [1, -1]

        // Test Case 4
        int[] nums4 = {9, 11};
        int k4 = 2;
        int[] output4 = maxSlidingWindow(nums4, k4);
        System.out.println("Test Case 4: " + Arrays.toString(output4)); // Expected: [11]

        // Test Case 5
        int[] nums5 = {7, 2, 4};
        int k5 = 2;
        int[] output5 = maxSlidingWindow(nums5, k5);
        System.out.println("Test Case 5: " + Arrays.toString(output5)); // Expected: [7, 4]
    }
}