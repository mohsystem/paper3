import java.util.Arrays;

public class Task161 {
    // 0/1 Knapsack: returns maximum value achievable within capacity.
    public static int knapsack(int capacity, int[] weights, int[] values) {
        // Input validation
        if (capacity < 0) {
            throw new IllegalArgumentException("Capacity must be non-negative.");
        }
        if (weights == null || values == null) {
            throw new IllegalArgumentException("Weights and values must not be null.");
        }
        if (weights.length != values.length) {
            throw new IllegalArgumentException("Weights and values must have the same length.");
        }

        int n = weights.length;
        for (int i = 0; i < n; i++) {
            if (weights[i] < 0) {
                throw new IllegalArgumentException("Weights must be non-negative.");
            }
            // Values can be any integers; if negatives appear, skipping items is still optimal
        }

        int[] dp = new int[capacity + 1];
        Arrays.fill(dp, 0);

        for (int i = 0; i < n; i++) {
            int wt = weights[i];
            int val = values[i];
            if (wt > capacity) {
                continue; // Item can't fit at all
            }
            for (int w = capacity; w >= wt; w--) {
                int candidate = dp[w - wt] + val;
                if (candidate > dp[w]) {
                    dp[w] = candidate;
                }
            }
        }
        return dp[capacity];
    }

    private static void runTest(int testNum, int capacity, int[] weights, int[] values, int expected) {
        int result = knapsack(capacity, weights, values);
        System.out.println("Test " + testNum + " -> result=" + result + " expected=" + expected);
    }

    public static void main(String[] args) {
        // Test 1: Classic example
        runTest(1, 50, new int[]{10, 20, 30}, new int[]{60, 100, 120}, 220);

        // Test 2: Empty items
        runTest(2, 10, new int[]{}, new int[]{}, 0);

        // Test 3: Zero capacity
        runTest(3, 0, new int[]{1, 2, 3}, new int[]{10, 20, 30}, 0);

        // Test 4: Items heavier than capacity
        runTest(4, 5, new int[]{6, 7, 8}, new int[]{10, 20, 30}, 0);

        // Test 5: Mixed items
        runTest(5, 7, new int[]{1, 3, 4, 5}, new int[]{1, 4, 5, 7}, 9);
    }
}