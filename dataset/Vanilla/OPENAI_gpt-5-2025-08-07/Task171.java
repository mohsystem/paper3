import java.util.*;

public class Task171 {
    public static long reversePairs(int[] nums) {
        if (nums == null || nums.length == 0) return 0L;
        int n = nums.length;
        int[] temp = new int[n];
        return sortCount(nums, 0, n - 1, temp);
    }

    private static long sortCount(int[] a, int l, int r, int[] temp) {
        if (l >= r) return 0L;
        int m = (l + r) >>> 1;
        long cnt = sortCount(a, l, m, temp) + sortCount(a, m + 1, r, temp);

        int j = m + 1;
        for (int i = l; i <= m; i++) {
            while (j <= r && (long) a[i] > 2L * (long) a[j]) j++;
            cnt += (j - (m + 1));
        }

        int i = l;
        j = m + 1;
        int k = l;
        while (i <= m && j <= r) {
            if (a[i] <= a[j]) temp[k++] = a[i++];
            else temp[k++] = a[j++];
        }
        while (i <= m) temp[k++] = a[i++];
        while (j <= r) temp[k++] = a[j++];

        for (int t = l; t <= r; t++) a[t] = temp[t];
        return cnt;
    }

    private static void printResult(int[] arr) {
        System.out.println(reversePairs(Arrays.copyOf(arr, arr.length)));
    }

    public static void main(String[] args) {
        printResult(new int[]{1, 3, 2, 3, 1});           // 2
        printResult(new int[]{2, 4, 3, 5, 1});           // 3
        printResult(new int[]{-5, -5});                  // 1
        printResult(new int[]{2147483647, -2147483648, 0}); // 2
        printResult(new int[]{5, 4, 3, 2, 1});           // 4
    }
}