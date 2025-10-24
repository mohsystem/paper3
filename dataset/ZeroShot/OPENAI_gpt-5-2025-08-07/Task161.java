import java.util.Arrays;

public class Task161 {
    private static final int MAX_CAPACITY = 2_000_000;

    // Returns the maximum value that fits into the knapsack.
    // capacity: non-negative capacity of the knapsack
    // weights, values: arrays of non-negative integers with equal length
    public static long maxKnapsackValue(int capacity, int[] weights, int[] values) {
        if (capacity < 0) return 0L;
        if (weights == null || values == null || weights.length != values.length) {
            throw new IllegalArgumentException("Weights and values must be non-null and of equal length.");
        }
        if (capacity > MAX_CAPACITY) {
            throw new IllegalArgumentException("Capacity too large.");
        }
        int n = weights.length;
        for (int i = 0; i < n; i++) {
            if (weights[i] < 0 || values[i] < 0) {
                throw new IllegalArgumentException("Weights and values must be non-negative.");
            }
        }
        long[] dp = new long[capacity + 1];
        for (int i = 0; i < n; i++) {
            int wt = weights[i];
            long val = values[i];
            if (wt <= capacity) {
                for (int c = capacity; c >= wt; c--) {
                    long base = dp[c - wt];
                    long candidate;
                    if (base > Long.MAX_VALUE - val) {
                        candidate = Long.MAX_VALUE;
                    } else {
                        candidate = base + val;
                    }
                    if (candidate > dp[c]) {
                        dp[c] = candidate;
                    }
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
        System.out.println(maxKnapsackValue(cap1, w1, v1)); // Expected 220

        // Test case 2
        int cap2 = 7;
        int[] w2 = {1, 3, 4, 5};
        int[] v2 = {1, 4, 5, 7};
        System.out.println(maxKnapsackValue(cap2, w2, v2)); // Expected 9

        // Test case 3
        int cap3 = 0;
        int[] w3 = {1, 2, 3};
        int[] v3 = {10, 15, 40};
        System.out.println(maxKnapsackValue(cap3, w3, v3)); // Expected 0

        // Test case 4
        int cap4 = 10;
        int[] w4 = {3, 4, 5, 9, 4};
        int[] v4 = {4, 5, 7, 10, 6};
        System.out.println(maxKnapsackValue(cap4, w4, v4)); // Expected 13

        // Test case 5
        int cap5 = 15;
        int[] w5 = {12, 2, 1, 1, 4};
        int[] v5 = {4, 2, 2, 1, 10};
        System.out.println(maxKnapsackValue(cap5, w5, v5)); // Expected 15
    }
}