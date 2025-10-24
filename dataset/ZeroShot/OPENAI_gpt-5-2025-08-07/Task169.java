import java.util.*;

public class Task169 {
    // Returns an array counts where counts[i] is number of smaller elements to the right of nums[i]
    public static int[] countSmaller(int[] nums) {
        if (nums == null) return new int[0];
        int n = nums.length;
        if (n == 0) return new int[0];

        // Coordinate compression
        int[] sorted = Arrays.stream(nums).distinct().sorted().toArray();
        Map<Integer, Integer> rank = new HashMap<>(sorted.length * 2);
        for (int i = 0; i < sorted.length; i++) {
            rank.put(sorted[i], i + 1); // 1-based index for Fenwick
        }

        int[] bit = new int[sorted.length + 1];
        int[] ans = new int[n];

        for (int i = n - 1; i >= 0; i--) {
            int r = rank.get(nums[i]);
            ans[i] = query(bit, r - 1);
            update(bit, r, 1);
        }
        return ans;
    }

    private static void update(int[] bit, int idx, int delta) {
        for (int i = idx; i < bit.length; i += i & -i) {
            bit[i] += delta;
        }
    }

    private static int query(int[] bit, int idx) {
        int sum = 0;
        for (int i = idx; i > 0; i -= i & -i) {
            sum += bit[i];
        }
        return sum;
    }

    private static String arrayToString(int[] a) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < a.length; i++) {
            if (i > 0) sb.append(',');
            sb.append(a[i]);
        }
        sb.append(']');
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {5, 2, 6, 1},
            {-1},
            {-1, -1},
            {3, 2, 2, 6, 1},
            {2, 0, 1}
        };
        for (int[] t : tests) {
            int[] res = countSmaller(t);
            System.out.println("Input: " + arrayToString(t));
            System.out.println("Output: " + arrayToString(res));
            System.out.println("---");
        }
    }
}