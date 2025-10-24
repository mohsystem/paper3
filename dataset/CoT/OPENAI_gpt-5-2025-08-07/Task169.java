import java.util.*;

public class Task169 {
    // Function: counts of smaller elements to the right
    public static int[] countSmaller(int[] nums) {
        if (nums == null) return new int[0];
        int n = nums.length;
        int[] result = new int[n];
        if (n == 0) return result;

        // Coordinate compression
        int[] sorted = Arrays.copyOf(nums, n);
        Arrays.sort(sorted);
        int[] unique = new int[n];
        int m = 0;
        for (int i = 0; i < n; i++) {
            if (i == 0 || sorted[i] != sorted[i - 1]) {
                unique[m++] = sorted[i];
            }
        }

        // Fenwick Tree (BIT)
        int[] bit = new int[m + 2];
        // Query prefix sum up to index i (1-based)
        java.util.function.IntUnaryOperator query = (idx) -> {
            int sum = 0;
            while (idx > 0) {
                sum += bit[idx];
                idx -= idx & -idx;
            }
            return sum;
        };
        // Update: add val at index i (1-based)
        java.util.function.BiConsumer<Integer, Integer> update = (idx, val) -> {
            int i = idx;
            while (i < bit.length) {
                bit[i] += val;
                i += i & -i;
            }
        };

        // Binary search to find 1-based index in unique array
        for (int i = n - 1; i >= 0; i--) {
            int x = nums[i];
            int l = 0, r = m - 1, pos = -1;
            while (l <= r) {
                int mid = l + ((r - l) >>> 1);
                if (unique[mid] >= x) {
                    if (unique[mid] == x) pos = mid;
                    r = mid - 1;
                } else {
                    l = mid + 1;
                }
            }
            int idx = pos + 1; // to 1-based
            result[i] = query.applyAsInt(idx - 1);
            update.accept(idx, 1);
        }

        return result;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // 5 test cases
        int[][] tests = new int[][]{
            {5, 2, 6, 1},
            {-1},
            {-1, -1},
            {1, 2, 3, 4},
            {4, 3, 2, 1}
        };

        for (int[] t : tests) {
            int[] res = countSmaller(t);
            printArray(res);
        }
    }
}