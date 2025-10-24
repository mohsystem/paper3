import java.util.Arrays;

public class Task171 {

    public static long reversePairs(int[] nums) {
        validateArray(nums);
        if (nums.length <= 1) return 0L;
        int[] temp = new int[nums.length];
        return sortAndCount(nums, temp, 0, nums.length - 1);
    }

    private static void validateArray(int[] nums) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        int n = nums.length;
        if (n < 1 || n > 50000) {
            throw new IllegalArgumentException("Array length must be in the range [1, 50000].");
        }
        // Elements are int by type; no additional range validation needed.
    }

    private static long sortAndCount(int[] nums, int[] temp, int left, int right) {
        if (left >= right) return 0L;
        int mid = left + (right - left) / 2;
        long count = 0L;
        count += sortAndCount(nums, temp, left, mid);
        count += sortAndCount(nums, temp, mid + 1, right);

        // Count cross reverse pairs
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            while (j <= right && (long) nums[i] > 2L * (long) nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        // Merge step
        int i = left;
        j = mid + 1;
        int k = left;
        while (i <= mid && j <= right) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        while (i <= mid) temp[k++] = nums[i++];
        while (j <= right) temp[k++] = nums[j++];
        for (i = left; i <= right; i++) nums[i] = temp[i];

        return count;
    }

    // Simple pretty print for arrays
    private static String arrayToString(int[] arr) {
        return Arrays.toString(arr);
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 3, 2, 3, 1},                 // expected 2
            {2, 4, 3, 5, 1},                 // expected 3
            {-5, -5},                        // expected 1
            {2147483647, -2147483648, 0},    // expected 2
            {1, 2, 3, 4, 5}                  // expected 0
        };

        for (int i = 0; i < tests.length; i++) {
            int[] copy = Arrays.copyOf(tests[i], tests[i].length);
            long res = reversePairs(copy);
            System.out.println("Test " + (i + 1) + " " + arrayToString(tests[i]) + " -> " + res);
        }
    }
}