import java.util.Arrays;

public class Task161 {

    /**
     * Solves the 0/1 knapsack problem.
     *
     * @param capacity The maximum weight capacity of the knapsack.
     * @param weights  An array of weights of the items.
     * @param values   An array of values of the items.
     * @return The maximum value that can be put into the knapsack, or -1 if input is invalid.
     */
    public static long knapsack(int capacity, int[] weights, int[] values) {
        // Input validation
        if (capacity < 0 || weights == null || values == null || weights.length != values.length) {
            return -1; // Invalid input
        }
        for (int w : weights) {
            if (w < 0) return -1;
        }
        for (int v : values) {
            if (v < 0) return -1;
        }

        int n = weights.length;
        if (n == 0 || capacity == 0) {
            return 0;
        }
        
        // DP table to store results of subproblems. Using long to prevent overflow.
        // dp[i][w] will be the maximum value that can be obtained with a capacity w using the first i items.
        long[][] dp = new long[n + 1][capacity + 1];

        // Build table dp[][] in bottom up manner
        for (int i = 0; i <= n; i++) {
            for (int w = 0; w <= capacity; w++) {
                if (i == 0 || w == 0) {
                    dp[i][w] = 0;
                } else {
                    // Current item's weight and value (i-1 is the index in original arrays)
                    int currentWeight = weights[i - 1];
                    int currentValue = values[i - 1];

                    if (currentWeight <= w) {
                        // Two choices:
                        // 1. Include the current item: its value + max value for remaining capacity
                        // 2. Exclude the current item: max value from previous items with same capacity
                        dp[i][w] = Math.max(currentValue + dp[i - 1][w - currentWeight], dp[i - 1][w]);
                    } else {
                        // Cannot include the current item as its weight exceeds current capacity
                        dp[i][w] = dp[i - 1][w];
                    }
                }
            }
        }

        return dp[n][capacity];
    }

    public static void main(String[] args) {
        // Test Case 1: Standard case
        int[] weights1 = {10, 20, 30};
        int[] values1 = {60, 100, 120};
        int capacity1 = 50;
        System.out.println("Test Case 1: Max value = " + knapsack(capacity1, weights1, values1)); // Expected: 220

        // Test Case 2: No items
        int[] weights2 = {};
        int[] values2 = {};
        int capacity2 = 10;
        System.out.println("Test Case 2: Max value = " + knapsack(capacity2, weights2, values2)); // Expected: 0

        // Test Case 3: Capacity too small for any item
        int[] weights3 = {10, 20};
        int[] values3 = {100, 200};
        int capacity3 = 5;
        System.out.println("Test Case 3: Max value = " + knapsack(capacity3, weights3, values3)); // Expected: 0

        // Test Case 4: All items fit
        int[] weights4 = {10, 20, 30};
        int[] values4 = {60, 100, 120};
        int capacity4 = 100;
        System.out.println("Test Case 4: Max value = " + knapsack(capacity4, weights4, values4)); // Expected: 280

        // Test Case 5: Complex case
        int[] weights5 = {5, 4, 6, 3};
        int[] values5 = {10, 40, 30, 50};
        int capacity5 = 10;
        System.out.println("Test Case 5: Max value = " + knapsack(capacity5, weights5, values5)); // Expected: 90
    }
}