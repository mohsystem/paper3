import java.util.Arrays;

public class Task171 {
    // Chain-of-Through:
    // 1) Problem understanding: count pairs (i<j) with nums[i] > 2*nums[j]
    // 2) Security: handle nulls, avoid integer overflow using long
    // 3) Secure coding: use safe indices, no unsafe operations
    // 4) Code review: use long for comparisons, stable merge
    // 5) Output: tested with 5 cases

    public static long reversePairs(int[] nums) {
        if (nums == null || nums.length < 2) return 0L;
        int[] temp = new int[nums.length];
        return sortAndCount(nums, 0, nums.length - 1, temp);
    }

    private static long sortAndCount(int[] a, int l, int r, int[] temp) {
        if (l >= r) return 0L;
        int m = l + (r - l) / 2;
        long count = 0L;
        count += sortAndCount(a, l, m, temp);
        count += sortAndCount(a, m + 1, r, temp);
        count += countPairs(a, l, m, r);
        merge(a, l, m, r, temp);
        return count;
    }

    private static long countPairs(int[] a, int l, int m, int r) {
        long cnt = 0L;
        int j = m + 1;
        for (int i = l; i <= m; i++) {
            while (j <= r && (long) a[i] > 2L * (long) a[j]) {
                j++;
            }
            cnt += (j - (m + 1));
        }
        return cnt;
    }

    private static void merge(int[] a, int l, int m, int r, int[] temp) {
        int i = l, j = m + 1, k = l;
        while (i <= m && j <= r) {
            if (a[i] <= a[j]) temp[k++] = a[i++];
            else temp[k++] = a[j++];
        }
        while (i <= m) temp[k++] = a[i++];
        while (j <= r) temp[k++] = a[j++];
        for (int t = l; t <= r; t++) a[t] = temp[t];
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
                {1, 3, 2, 3, 1},
                {2, 4, 3, 5, 1},
                {5, 4, 3, 2, 1},
                {1, 1, 1, 1},
                {-5, -5}
        };
        for (int i = 0; i < tests.length; i++) {
            int[] arr = Arrays.copyOf(tests[i], tests[i].length);
            long res = reversePairs(arr);
            System.out.println(res);
        }
    }
}