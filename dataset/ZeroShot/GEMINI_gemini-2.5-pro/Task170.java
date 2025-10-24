import java.util.Arrays;

public class Task170 {

    /**
     * Given an integer array nums and two integers lower and upper, return the number of range sums
     * that lie in [lower, upper] inclusive.
     *
     * @param nums   The input integer array.
     * @param lower  The lower bound of the range sum.
     * @param upper  The upper bound of the range sum.
     * @return The number of range sums in the inclusive range [lower, upper].
     */
    public int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        int n = nums.length;
        // Prefix sums can exceed Integer.MAX_VALUE, so use long.
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }
        return mergeSortAndCount(prefixSums, 0, n, lower, upper);
    }

    /**
     * A recursive helper function using a divide and conquer approach (Merge Sort).
     * It counts the valid range sums and sorts the prefix sum subarray.
     *
     * @param sums   The array of prefix sums.
     * @param start  The starting index of the subarray (inclusive).
     * @param end    The ending index of the subarray (inclusive).
     * @param lower  The lower bound of the target range.
     * @param upper  The upper bound of the target range.
     * @return The count of valid range sums within this subarray and across its halves.
     */
    private int mergeSortAndCount(long[] sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }
        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(sums, start, mid, lower, upper) +
                    mergeSortAndCount(sums, mid + 1, end, lower, upper);

        // Count range sums S(i, j) where i is in the left half and j in the right half.
        // For each sums[j] in the right half, we need to find how many sums[i] in the left half
        // satisfy: lower <= sums[j] - sums[i] <= upper
        // which is equivalent to: sums[j] - upper <= sums[i] <= sums[j] - lower
        int k = start, l = start;
        for (int j = mid + 1; j <= end; j++) {
            while (k <= mid && sums[k] < sums[j] - upper) {
                k++;
            }
            while (l <= mid && sums[l] <= sums[j] - lower) {
                l++;
            }
            count += l - k;
        }

        // Standard merge step for merge sort
        long[] temp = new long[end - start + 1];
        int i = start, j = mid + 1, t = 0;
        while (i <= mid && j <= end) {
            if (sums[i] <= sums[j]) {
                temp[t++] = sums[i++];
            } else {
                temp[t++] = sums[j++];
            }
        }
        while (i <= mid) {
            temp[t++] = sums[i++];
        }
        while (j <= end) {
            temp[t++] = sums[j++];
        }
        System.arraycopy(temp, 0, sums, start, temp.length);
        
        return count;
    }

    public static void main(String[] args) {
        Task170 solution = new Task170();

        // Test Case 1
        int[] nums1 = {-2, 5, -1};
        int lower1 = -2, upper1 = 2;
        System.out.println("Test Case 1: " + solution.countRangeSum(nums1, lower1, upper1));

        // Test Case 2
        int[] nums2 = {0};
        int lower2 = 0, upper2 = 0;
        System.out.println("Test Case 2: " + solution.countRangeSum(nums2, lower2, upper2));

        // Test Case 3
        int[] nums3 = {1, 1, 1};
        int lower3 = 1, upper3 = 2;
        System.out.println("Test Case 3: " + solution.countRangeSum(nums3, lower3, upper3));

        // Test Case 4
        int[] nums4 = {-1, -1, -1};
        int lower4 = -2, upper4 = -1;
        System.out.println("Test Case 4: " + solution.countRangeSum(nums4, lower4, upper4));

        // Test Case 5
        int[] nums5 = {2147483647, -2147483648, -1, 0};
        int lower5 = -1, upper5 = 0;
        System.out.println("Test Case 5: " + solution.countRangeSum(nums5, lower5, upper5));
    }
}