import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Deque;

public class Task179 {
    public static int[] maxSlidingWindow(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            return new int[0];
        }
        int n = nums.length;
        int[] res = new int[n - k + 1];
        Deque<Integer> dq = new ArrayDeque<>();
        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && dq.peekFirst() <= i - k) {
                dq.pollFirst();
            }
            while (!dq.isEmpty() && nums[dq.peekLast()] <= nums[i]) {
                dq.pollLast();
            }
            dq.offerLast(i);
            if (i >= k - 1) {
                res[i - k + 1] = nums[dq.peekFirst()];
            }
        }
        return res;
    }

    private static void printArray(int[] arr) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) sb.append(',');
            sb.append(arr[i]);
        }
        sb.append(']');
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        int[][] tests = {
            {1,3,-1,-3,5,3,6,7},
            {1},
            {9,11},
            {4,-2},
            {7,2,4}
        };
        int[] ks = {3,1,2,1,2};

        for (int i = 0; i < tests.length; i++) {
            int[] out = maxSlidingWindow(tests[i], ks[i]);
            printArray(out);
        }
    }
}