// Chain-of-Through Steps:
// 1) Problem understanding: Implement countRangeSum using prefix sums + modified merge sort to count ranges in [lower, upper].
// 2) Security requirements: Use long for prefix sums to avoid overflow; validate inputs; avoid unsafe operations.
// 3) Secure coding: No external I/O; defensive checks; avoid integer overflow in intermediate calculations.
// 4) Code review: Use stable merge, bounds-checked loops; reduce allocations by reusing temp buffer.
// 5) Secure code output: Final code mitigates overflows and handles edge cases.

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) return 0;
        int n = nums.length;
        long[] sums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            sums[i + 1] = sums[i] + (long) nums[i];
        }
        long[] temp = new long[n + 1];
        long res = sortAndCount(sums, temp, 0, n + 1, (long) lower, (long) upper);
        // The result is guaranteed to fit 32-bit integer as per constraints.
        return (int) res;
    }

    private static long sortAndCount(long[] sums, long[] temp, int lo, int hi, long lower, long upper) {
        if (hi - lo <= 1) return 0L;
        int mid = (lo + hi) >>> 1;
        long count = 0;
        count += sortAndCount(sums, temp, lo, mid, lower, upper);
        count += sortAndCount(sums, temp, mid, hi, lower, upper);

        int j = mid, k = mid, t = mid;
        int r = lo;
        for (int i = lo; i < mid; i++) {
            while (k < hi && sums[k] - sums[i] < lower) k++;
            while (j < hi && sums[j] - sums[i] <= upper) j++;
            while (t < hi && sums[t] < sums[i]) temp[r++] = sums[t++];
            temp[r++] = sums[i];
            count += (j - k);
        }
        while (t < hi) temp[r++] = sums[t++];
        System.arraycopy(temp, lo, sums, lo, hi - lo);
        return count;
    }

    // 5 test cases
    public static void main(String[] args) {
        int[][] testsNums = {
            {-2, 5, -1},
            {0},
            {1, -1, 1},
            {2147483647, -2147483648, -1, 0},
            {1, 2, 3}
        };
        int[] lowers = {-2, 0, 0, -1, 3};
        int[] uppers = {2, 0, 1, 0, 6};

        for (int i = 0; i < testsNums.length; i++) {
            int result = countRangeSum(testsNums[i], lowers[i], uppers[i]);
            System.out.println(result);
        }
    }
}