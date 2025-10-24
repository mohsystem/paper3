import java.util.*;

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        int n = nums.length;
        long[] sums = new long[n + 1];
        for (int i = 0; i < n; i++) sums[i + 1] = sums[i] + nums[i];
        long[] temp = new long[n + 1];
        return (int) sortCount(sums, 0, n + 1, lower, upper, temp);
    }

    private static long sortCount(long[] sums, int lo, int hi, long lower, long upper, long[] temp) {
        if (hi - lo <= 1) return 0;
        int mid = (lo + hi) >>> 1;
        long count = sortCount(sums, lo, mid, lower, upper, temp) + sortCount(sums, mid, hi, lower, upper, temp);
        int l = mid, r = mid;
        for (int i = lo; i < mid; i++) {
            while (l < hi && sums[l] - sums[i] < lower) l++;
            while (r < hi && sums[r] - sums[i] <= upper) r++;
            count += r - l;
        }
        int i = lo, j = mid, k = lo;
        while (i < mid && j < hi) {
            if (sums[i] <= sums[j]) temp[k++] = sums[i++];
            else temp[k++] = sums[j++];
        }
        while (i < mid) temp[k++] = sums[i++];
        while (j < hi) temp[k++] = sums[j++];
        for (i = lo; i < hi; i++) sums[i] = temp[i];
        return count;
    }

    public static void main(String[] args) {
        int[][] testsNums = {
                {-2, 5, -1},
                {0},
                {1, -1, 2, -2, 3},
                {0, 0, 0},
                {2, -2, 2, -2}
        };
        int[] lowers = {-2, 0, 1, 0, -1};
        int[] uppers = {2, 0, 3, 0, 1};
        for (int t = 0; t < testsNums.length; t++) {
            System.out.println(countRangeSum(testsNums[t], lowers[t], uppers[t]));
        }
    }
}