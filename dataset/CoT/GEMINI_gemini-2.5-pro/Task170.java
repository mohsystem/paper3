public class Task170 {

    private int mergeSortAndCount(long[] sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(sums, start, mid, lower, upper) +
                    mergeSortAndCount(sums, mid + 1, end, lower, upper);

        int l = mid + 1;
        int r = mid + 1;
        for (int i = start; i <= mid; i++) {
            while (l <= end && sums[l] - sums[i] < lower) {
                l++;
            }
            while (r <= end && sums[r] - sums[i] <= upper) {
                r++;
            }
            count += (r - l);
        }

        // Merge step
        long[] temp = new long[end - start + 1];
        int p1 = start, p2 = mid + 1, p = 0;
        while (p1 <= mid && p2 <= end) {
            if (sums[p1] <= sums[p2]) {
                temp[p++] = sums[p1++];
            } else {
                temp[p++] = sums[p2++];
            }
        }
        while (p1 <= mid) {
            temp[p++] = sums[p1++];
        }
        while (p2 <= end) {
            temp[p++] = sums[p2++];
        }
        System.arraycopy(temp, 0, sums, start, temp.length);

        return count;
    }

    public int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        int n = nums.length;
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        return mergeSortAndCount(prefixSums, 0, n, lower, upper);
    }

    public static void main(String[] args) {
        Task170 solution = new Task170();

        // Test Case 1
        int[] nums1 = {-2, 5, -1};
        int lower1 = -2;
        int upper1 = 2;
        System.out.println("Test Case 1: " + solution.countRangeSum(nums1, lower1, upper1)); // Expected: 3

        // Test Case 2
        int[] nums2 = {0};
        int lower2 = 0;
        int upper2 = 0;
        System.out.println("Test Case 2: " + solution.countRangeSum(nums2, lower2, upper2)); // Expected: 1

        // Test Case 3
        int[] nums3 = {1, -1, 1, -1};
        int lower3 = 0;
        int upper3 = 0;
        System.out.println("Test Case 3: " + solution.countRangeSum(nums3, lower3, upper3)); // Expected: 6

        // Test Case 4
        int[] nums4 = {Integer.MAX_VALUE, Integer.MIN_VALUE, 0, -1, 1};
        int lower4 = -1;
        int upper4 = 1;
        System.out.println("Test Case 4: " + solution.countRangeSum(nums4, lower4, upper4)); // Expected: 9

        // Test Case 5
        int[] nums5 = {0, 0, 0, 0, 0};
        int lower5 = 0;
        int upper5 = 0;
        System.out.println("Test Case 5: " + solution.countRangeSum(nums5, lower5, upper5)); // Expected: 15
    }
}