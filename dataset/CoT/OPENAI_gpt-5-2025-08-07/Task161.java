import java.util.Arrays;

public class Task161 {
    public static long knapsack(int capacity, int[] weights, int[] values) {
        if (capacity <= 0 || weights == null || values == null || weights.length != values.length) {
            return 0L;
        }
        final int n = weights.length;
        long[] dp = new long[capacity + 1];
        Arrays.fill(dp, 0L);

        for (int i = 0; i < n; i++) {
            int w = weights[i];
            int v = values[i];
            if (w <= 0 || v < 0) {
                continue; // Skip invalid items to avoid undefined behavior
            }
            for (int c = capacity; c >= w; c--) {
                long candidate = dp[c - w] + (long) v;
                if (candidate > dp[c]) {
                    dp[c] = candidate;
                }
            }
        }
        return dp[capacity];
    }

    public static void main(String[] args) {
        // Test case 1
        int cap1 = 50;
        int[] w1 = {10, 20, 30};
        int[] v1 = {60, 100, 120};
        System.out.println(knapsack(cap1, w1, v1)); // Expected: 220

        // Test case 2
        int cap2 = 0;
        int[] w2 = {5, 10};
        int[] v2 = {10, 20};
        System.out.println(knapsack(cap2, w2, v2)); // Expected: 0

        // Test case 3
        int cap3 = 7;
        int[] w3 = {1, 3, 4, 5};
        int[] v3 = {1, 4, 5, 7};
        System.out.println(knapsack(cap3, w3, v3)); // Expected: 9

        // Test case 4
        int cap4 = 10;
        int[] w4 = {6, 3, 4, 2};
        int[] v4 = {30, 14, 16, 9};
        System.out.println(knapsack(cap4, w4, v4)); // Expected: 46

        // Test case 5
        int cap5 = 5;
        int[] w5 = {4, 2, 3};
        int[] v5 = {10, 4, 7};
        System.out.println(knapsack(cap5, w5, v5)); // Expected: 11
    }
}