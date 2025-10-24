import java.util.*;

public class Task161 {
    public static int knapsack(int capacity, int[] weights, int[] values) {
        if (weights == null || values == null || weights.length != values.length || capacity <= 0 || weights.length == 0) {
            return 0;
        }
        int n = weights.length;
        int[] dp = new int[Math.max(0, capacity) + 1];
        for (int i = 0; i < n; i++) {
            int w = weights[i];
            int v = values[i];
            for (int c = capacity; c >= w; c--) {
                dp[c] = Math.max(dp[c], dp[c - w] + v);
            }
        }
        return dp[capacity];
    }

    public static void main(String[] args) {
        // Test case 1
        int cap1 = 5;
        int[] w1 = {1, 2, 3};
        int[] v1 = {6, 10, 12};
        System.out.println(knapsack(cap1, w1, v1)); // Expected 22

        // Test case 2
        int cap2 = 3;
        int[] w2 = {4, 5};
        int[] v2 = {7, 8};
        System.out.println(knapsack(cap2, w2, v2)); // Expected 0

        // Test case 3
        int cap3 = 0;
        int[] w3 = {1, 2};
        int[] v3 = {10, 20};
        System.out.println(knapsack(cap3, w3, v3)); // Expected 0

        // Test case 4
        int cap4 = 4;
        int[] w4 = {4};
        int[] v4 = {10};
        System.out.println(knapsack(cap4, w4, v4)); // Expected 10

        // Test case 5
        int cap5 = 10;
        int[] w5 = {2, 5, 1, 3, 4};
        int[] v5 = {3, 4, 7, 8, 9};
        System.out.println(knapsack(cap5, w5, v5)); // Expected 27
    }
}