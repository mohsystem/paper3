import java.util.Arrays;

public class Task145 {
    public static long maxSubarraySum(int[] nums) {
        if (nums == null || nums.length == 0) {
            return 0L;
        }
        long current = nums[0];
        long best = nums[0];
        for (int i = 1; i < nums.length; i++) {
            long val = nums[i];
            current = Math.max(val, current + val);
            best = Math.max(best, current);
        }
        return best;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{-2, 1, -3, 4, -1, 2, 1, -5, 4},
            new int[]{1, 2, 3, 4},
            new int[]{-1, -2, -3},
            new int[]{5, -2, 3, -1, 2},
            new int[]{} // empty array
        };

        for (int i = 0; i < tests.length; i++) {
            long result = maxSubarraySum(tests[i]);
            System.out.println("Test " + (i + 1) + " input: " + Arrays.toString(tests[i]) + " -> Max Sum: " + result);
        }
    }
}