import java.util.*;

public class Task171 {
    public static long countReversePairs(int[] nums) {
        if (nums == null || nums.length <= 1) return 0L;
        int n = nums.length;
        long[] arr = new long[n];
        for (int i = 0; i < n; i++) {
            arr[i] = nums[i];
        }
        long[] temp = new long[n];
        return mergeSortCount(arr, temp, 0, n - 1);
    }

    private static long mergeSortCount(long[] arr, long[] temp, int left, int right) {
        if (left >= right) return 0L;
        int mid = left + (right - left) / 2;
        long count = 0L;
        count += mergeSortCount(arr, temp, left, mid);
        count += mergeSortCount(arr, temp, mid + 1, right);

        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            while (j <= right && arr[i] > 2L * arr[j]) j++;
            count += (j - (mid + 1));
        }

        // Merge step
        int i = left;
        j = mid + 1;
        int k = left;
        while (i <= mid && j <= right) {
            if (arr[i] <= arr[j]) temp[k++] = arr[i++];
            else temp[k++] = arr[j++];
        }
        while (i <= mid) temp[k++] = arr[i++];
        while (j <= right) temp[k++] = arr[j++];
        for (int idx = left; idx <= right; idx++) arr[idx] = temp[idx];

        return count;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 3, 2, 3, 1},
            {2, 4, 3, 5, 1},
            {1},
            {2147483647, -2147483648, 0},
            {-5, -5}
        };
        for (int t = 0; t < tests.length; t++) {
            long res = countReversePairs(tests[t]);
            System.out.println("Test " + (t + 1) + " result: " + res);
        }
    }
}