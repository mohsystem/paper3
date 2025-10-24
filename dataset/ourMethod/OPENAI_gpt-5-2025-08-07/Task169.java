import java.util.*;

public class Task169 {
    // Counts the number of smaller elements to the right of each element in nums.
    // Returns an array of counts. Validates inputs and throws IllegalArgumentException on invalid data.
    public static int[] countSmaller(int[] nums) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array is null.");
        }
        int n = nums.length;
        if (n < 0 || n > 100000) {
            throw new IllegalArgumentException("Invalid array length.");
        }
        final int MIN_VAL = -10000;
        final int MAX_VAL = 10000;
        for (int v : nums) {
            if (v < MIN_VAL || v > MAX_VAL) {
                throw new IllegalArgumentException("Value out of allowed range.");
            }
        }
        if (n == 0) return new int[0];

        // Coordinate compression
        int[] sorted = Arrays.copyOf(nums, n);
        Arrays.sort(sorted);
        int[] uniq = new int[n];
        int m = 0;
        for (int v : sorted) {
            if (m == 0 || uniq[m - 1] != v) {
                uniq[m++] = v;
            }
        }

        int[] bit = new int[m + 2];
        int[] res = new int[n];

        for (int i = n - 1; i >= 0; i--) {
            int idx = lowerBound(uniq, m, nums[i]) + 1; // 1-based for BIT
            res[i] = query(bit, idx - 1);
            add(bit, idx, 1);
        }
        return res;
    }

    private static int lowerBound(int[] arr, int len, int target) {
        int l = 0, r = len;
        while (l < r) {
            int mid = l + ((r - l) >>> 1);
            if (arr[mid] < target) l = mid + 1;
            else r = mid;
        }
        return l;
    }

    private static void add(int[] bit, int idx, int delta) {
        while (idx < bit.length) {
            bit[idx] += delta;
            idx += idx & -idx;
        }
    }

    private static int query(int[] bit, int idx) {
        int sum = 0;
        while (idx > 0) {
            sum += bit[idx];
            idx -= idx & -idx;
        }
        return sum;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {5, 2, 6, 1},
            {-1},
            {-1, -1},
            {1, 2, 3},
            {3, 2, 1}
        };

        for (int[] t : tests) {
            int[] ans = countSmaller(t);
            printArray(ans);
        }
    }
}