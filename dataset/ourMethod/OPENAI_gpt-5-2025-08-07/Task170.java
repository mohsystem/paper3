import java.util.Arrays;

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null) {
            throw new IllegalArgumentException("nums must not be null");
        }
        int n = nums.length;
        if (n < 1 || n > 100000) {
            throw new IllegalArgumentException("nums length out of allowed range");
        }
        if (lower > upper) {
            throw new IllegalArgumentException("lower must be <= upper");
        }
        if (lower < -100000 || upper > 100000) {
            throw new IllegalArgumentException("lower/upper out of allowed range");
        }
        long[] sums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            sums[i + 1] = sums[i] + (long) nums[i];
        }
        long[] temp = new long[n + 1];
        long count = sortCount(sums, 0, n + 1, lower, upper, temp);
        if (count < Integer.MIN_VALUE || count > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Result out of 32-bit integer range");
        }
        return (int) count;
    }

    private static long sortCount(long[] sums, int left, int right, long lower, long upper, long[] temp) {
        if (right - left <= 1) return 0L;
        int mid = left + (right - left) / 2;
        long cnt = 0L;
        cnt += sortCount(sums, left, mid, lower, upper, temp);
        cnt += sortCount(sums, mid, right, lower, upper, temp);

        int lo = mid, hi = mid;
        for (int i = left; i < mid; i++) {
            while (lo < right && sums[lo] - sums[i] < lower) lo++;
            while (hi < right && sums[hi] - sums[i] <= upper) hi++;
            cnt += (hi - lo);
        }
        // Merge step
        int i = left, j = mid, k = left;
        while (i < mid || j < right) {
            if (j >= right || (i < mid && sums[i] <= sums[j])) {
                temp[k++] = sums[i++];
            } else {
                temp[k++] = sums[j++];
            }
        }
        for (i = left; i < right; i++) {
            sums[i] = temp[i];
        }
        return cnt;
    }

    private static void runTest(int[] nums, int lower, int upper, int expected) {
        int result = countRangeSum(nums, lower, upper);
        System.out.println("Result: " + result + " | Expected: " + expected);
    }

    public static void main(String[] args) {
        // Five test cases
        runTest(new int[]{-2, 5, -1}, -2, 2, 3);
        runTest(new int[]{0}, 0, 0, 1);
        runTest(new int[]{1, 2, 3}, 3, 6, 4);
        runTest(new int[]{-1, -1, -1}, -3, -1, 6);
        runTest(new int[]{1, -1, 1, -1}, 0, 0, 4);
    }
}