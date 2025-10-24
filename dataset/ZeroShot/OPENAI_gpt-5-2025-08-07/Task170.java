import java.util.Arrays;

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null) return 0;
        final int n = nums.length;
        long[] prefix = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = prefix[i] + (long) nums[i];
        }
        long[] temp = new long[n + 1];
        long count = sortAndCount(prefix, temp, 0, n + 1, (long) lower, (long) upper);
        if (count > Integer.MAX_VALUE) return Integer.MAX_VALUE;
        if (count < Integer.MIN_VALUE) return Integer.MIN_VALUE;
        return (int) count;
    }

    private static long sortAndCount(long[] sums, long[] temp, int left, int right, long lower, long upper) {
        if (right - left <= 1) return 0L;
        int mid = left + ((right - left) >>> 1);
        long count = 0L;
        count += sortAndCount(sums, temp, left, mid, lower, upper);
        count += sortAndCount(sums, temp, mid, right, lower, upper);

        int j = mid, k = mid, r = mid, p = left;
        for (int i = left; i < mid; i++) {
            while (j < right && sums[j] - sums[i] < lower) j++;
            while (k < right && sums[k] - sums[i] <= upper) k++;
            count += (k - j);
            while (r < right && sums[r] < sums[i]) {
                temp[p++] = sums[r++];
            }
            temp[p++] = sums[i];
        }
        while (r < right) {
            temp[p++] = sums[r++];
        }
        System.arraycopy(temp, left, sums, left, right - left);
        return count;
    }

    public static void main(String[] args) {
        int[][] testNums = {
            {-2, 5, -1},
            {0},
            {1, 2, 3, 4},
            {-1, -1, -1},
            {0, 0, 0, 0}
        };
        int[] lowers = {-2, 0, 3, -2, 0};
        int[] uppers = {2, 0, 6, -1, 0};

        for (int i = 0; i < testNums.length; i++) {
            System.out.println(countRangeSum(testNums[i], lowers[i], uppers[i]));
        }
    }
}