import java.util.ArrayDeque;
import java.util.Arrays;

public class Task179 {
    public static int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || k <= 0 || nums.length == 0 || k > nums.length) {
            return new int[0];
        }
        final int n = nums.length;
        int[] result = new int[n - k + 1];
        ArrayDeque<Integer> dq = new ArrayDeque<>(); // store indices, values in decreasing order

        for (int i = 0; i < n; i++) {
            // Remove indices out of the current window
            while (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }
            // Maintain decreasing deque by values
            while (!dq.isEmpty() && nums[dq.peekLast()] <= nums[i]) {
                dq.pollLast();
            }
            dq.addLast(i);

            if (i >= k - 1) {
                result[i - k + 1] = nums[dq.peekFirst()];
            }
        }
        return result;
    }

    private static void runTest(int[] nums, int k, int[] expected) {
        int[] out = maxSlidingWindow(nums, k);
        System.out.println("nums=" + Arrays.toString(nums) + ", k=" + k);
        System.out.println("out =" + Arrays.toString(out));
        System.out.println("exp =" + Arrays.toString(expected));
        System.out.println("---");
    }

    public static void main(String[] args) {
        runTest(new int[]{1,3,-1,-3,5,3,6,7}, 3, new int[]{3,3,5,5,6,7});
        runTest(new int[]{1}, 1, new int[]{1});
        runTest(new int[]{9,8,7,6}, 4, new int[]{9});
        runTest(new int[]{1,2,3,4,5}, 2, new int[]{2,3,4,5});
        runTest(new int[]{5,4,3,2,1}, 2, new int[]{5,4,3,2});
    }
}