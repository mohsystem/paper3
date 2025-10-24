// Task145 - Java implementation
// Chain-of-Through process:
// 1) Problem: Given an array of integers, return the maximum sum of any contiguous subarray.
// 2) Security: Avoid overflows by using long for accumulation; handle null/empty input safely.
// 3) Secure coding: No external input; pure function that validates inputs.
// 4) Code review: Checked for null, empty, and all-negative arrays; uses long to prevent overflow.
// 5) Final secure output: Returns 0 for null/empty arrays.

public class Task145 {

    public static long maxSubArray(int[] nums) {
        if (nums == null || nums.length == 0) {
            return 0L;
        }
        long best = Long.MIN_VALUE;
        long curr = 0L;
        for (int x : nums) {
            long lx = (long) x;
            curr = Math.max(lx, curr + lx);
            if (curr > best) {
                best = curr;
            }
        }
        return best;
    }

    private static void runTest(int[] nums) {
        long res = maxSubArray(nums);
        System.out.println(res);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(new int[]{1, -3, 2, 1, -1});      // Expected 3
        runTest(new int[]{-2, -3, -1, -4});       // Expected -1
        runTest(new int[]{5});                    // Expected 5
        runTest(new int[]{});                     // Expected 0
        runTest(new int[]{4, -1, 2, 1, -5, 4});   // Expected 6
    }
}