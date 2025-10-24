import java.util.Arrays;

class Task170 {
    public int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        int n = nums.length;
        long[] prefixSums = new long[n + 1];
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }
        return countAndMerge(prefixSums, 0, n, lower, upper);
    }

    private int countAndMerge(long[] sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = countAndMerge(sums, start, mid, lower, upper) + countAndMerge(sums, mid + 1, end, lower, upper);

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

        // Merge step
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
        Task170 solver = new Task170();
        
        // Test case 1
        int[] nums1 = {-2, 5, -1};
        int lower1 = -2;
        int upper1 = 2;
        System.out.println("Test Case 1: " + solver.countRangeSum(nums1, lower1, upper1));

        // Test case 2
        int[] nums2 = {0};
        int lower2 = 0;
        int upper2 = 0;
        System.out.println("Test Case 2: " + solver.countRangeSum(nums2, lower2, upper2));

        // Test case 3
        int[] nums3 = {2147483647, -2147483648, -1, 0};
        int lower3 = -1;
        int upper3 = 0;
        System.out.println("Test Case 3: " + solver.countRangeSum(nums3, lower3, upper3));

        // Test case 4
        int[] nums4 = {0, 0, 0};
        int lower4 = 0;
        int upper4 = 0;
        System.out.println("Test Case 4: " + solver.countRangeSum(nums4, lower4, upper4));

        // Test case 5
        int[] nums5 = {-1, 1};
        int lower5 = 0;
        int upper5 = 0;
        System.out.println("Test Case 5: " + solver.countRangeSum(nums5, lower5, upper5));
    }
}