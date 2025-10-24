import java.util.Arrays;

public class Task161 {

    /**
     * Solves the 0/1 knapsack problem using dynamic programming.
     *
     * @param capacity The maximum weight capacity of the knapsack.
     * @param weights  An array of weights of the items.
     * @param values   An array of values of the items.
     * @return The maximum value that can be put into the knapsack.
     */
    public static int knapsack(int capacity, int[] weights, int[] values) {
        // --- Secure Code: Input Validation ---
        if (capacity < 0 || weights == null || values == null || weights.length != values.length) {
            // Invalid arguments, returning 0 as a safe default.
            // In a real-world application, throwing IllegalArgumentException would be better.
            return 0;
        }

        int n = values.length;
        if (n == 0 || capacity == 0) {
            return 0;
        }

        // dp[w] will be the maximum value that can be obtained for a knapsack of capacity w
        int[] dp = new int[capacity + 1];

        // Build table dp[] in a bottom-up manner
        for (int i = 0; i < n; i++) {
            // --- Secure Code: Check for invalid item properties ---
            if (weights[i] < 0 || values[i] < 0) {
                // Skip items with negative weight or value, which are nonsensical for this problem.
                continue;
            }
            
            // Traverse backwards to ensure each item is considered only once (0/1 property)
            for (int w = capacity; w >= weights[i]; w--) {
                dp[w] = Math.max(dp[w], values[i] + dp[w - weights[i]]);
            }
        }

        return dp[capacity];
    }

    public static void main(String[] args) {
        // Test Case 1: General case
        int capacity1 = 50;
        int[] values1 = {60, 100, 120};
        int[] weights1 = {10, 20, 30};
        System.out.println("Test Case 1: Max value = " + knapsack(capacity1, weights1, values1));

        // Test Case 2: Empty items
        int capacity2 = 10;
        int[] values2 = {};
        int[] weights2 = {};
        System.out.println("Test Case 2: Max value = " + knapsack(capacity2, weights2, values2));

        // Test Case 3: Zero capacity
        int capacity3 = 0;
        int[] values3 = {60, 100};
        int[] weights3 = {10, 20};
        System.out.println("Test Case 3: Max value = " + knapsack(capacity3, weights3, values3));

        // Test Case 4: Items that partially fit
        int capacity4 = 5;
        int[] values4 = {3, 4, 5, 6};
        int[] weights4 = {2, 3, 4, 5};
        System.out.println("Test Case 4: Max value = " + knapsack(capacity4, weights4, values4));

        // Test Case 5: More complex case
        int capacity5 = 10;
        int[] values5 = {10, 40, 30, 50};
        int[] weights5 = {5, 4, 6, 3};
        System.out.println("Test Case 5: Max value = " + knapsack(capacity5, weights5, values5));
    }
}