import java.util.*;

public class Task146 {
    public static int findMissing(int[] arr, int n) {
        if (n < 1 || arr == null || arr.length != n - 1) {
            return -1;
        }
        boolean[] seen = new boolean[n + 1]; // index 0 unused
        int xor = 0;
        for (int i = 1; i <= n; i++) {
            xor ^= i;
        }
        for (int v : arr) {
            if (v < 1 || v > n) {
                return -1;
            }
            if (seen[v]) {
                return -1;
            }
            seen[v] = true;
            xor ^= v;
        }
        if (xor < 1 || xor > n || seen[xor]) {
            return -1;
        }
        return xor;
    }

    public static void main(String[] args) {
        int[] t1 = new int[]{1, 2, 3, 5};        // n=5 -> 4
        int[] t2 = new int[]{};                  // n=1 -> 1
        int[] t3 = new int[]{7, 6, 5, 4, 3, 2};  // n=7 -> 1
        int[] t4 = new int[]{1, 2, 2};           // n=4 -> -1 (duplicate)
        int[] t5 = new int[]{0, 1, 2, 3, 4};     // n=6 -> -1 (out of range)

        int[] ns = new int[]{5, 1, 7, 4, 6};
        int[][] tests = new int[][]{t1, t2, t3, t4, t5};

        for (int i = 0; i < tests.length; i++) {
            System.out.println(findMissing(tests[i], ns[i]));
        }
    }
}