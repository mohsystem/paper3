public class Task170 {

    private int mergeSortAndCount(long[] sums, int start, int end, int lower, int upper) {
        if (end - start <= 1) {
            return 0;
        }
        
        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(sums, start, mid, lower, upper)
                  + mergeSortAndCount(sums, mid, end, lower, upper);
        
        int l = mid, r = mid;
        for (int i = start; i < mid; i++) {
            while (l < end && sums[l] - sums[i] < lower) {
                l++;
            }
            while (r < end && sums[r] - sums[i] <= upper) {
                r++;
            }
            count += r - l;
        }
        
        long[] temp = new long[end - start];
        int i = start, j = mid, t = 0;
        while (i < mid && j < end) {
            if (sums[i] <= sums[j]) {
                temp[t++] = sums[i++];
            } else {
                temp[t++] = sums[j++];
            }
        }
        while (i < mid) {
            temp[t++] = sums[i++];
        }
        while (j < end) {
            temp[t++] = sums[j++];
        }
        System.arraycopy(temp, 0, sums, start, end - start);
        
        return count;
    }

    public int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        int n = nums.length;
        long[] prefixSums = new long[n + 1];
        prefixSums[0] = 0;
        for (int i = 0; i < n; i++) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }
        
        return mergeSortAndCount(prefixSums, 0, n + 1, lower, upper);
    }

    public static void main(String[] args) {
        Task170 solver = new Task170();
        
        // Test case 1
        int[] nums1 = {-2, 5, -1};
        int lower1 = -2, upper1 = 2;
        System.out.println("Test 1: " + solver.countRangeSum(nums1, lower1, upper1));

        // Test case 2
        int[] nums2 = {0};
        int lower2 = 0, upper2 = 0;
        System.out.println("Test 2: " + solver.countRangeSum(nums2, lower2, upper2));

        // Test case 3
        int[] nums3 = {2147483647, -2147483647, -1, 0};
        int lower3 = -1, upper3 = 0;
        System.out.println("Test 3: " + solver.countRangeSum(nums3, lower3, upper3));

        // Test case 4
        int[] nums4 = {0, 0, 0};
        int lower4 = 0, upper4 = 0;
        System.out.println("Test 4: " + solver.countRangeSum(nums4, lower4, upper4));

        // Test case 5
        int[] nums5 = {-1, 1};
        int lower5 = 0, upper5 = 0;
        System.out.println("Test 5: " + solver.countRangeSum(nums5, lower5, upper5));
    }
}