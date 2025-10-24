public class Task161 {

    /**
     * Solves the 0/1 knapsack problem using dynamic programming.
     *
     * @param capacity The maximum weight capacity of the knapsack.
     * @param weights  An array of weights of the items.
     * @param values   An array of values of the items.
     * @param n        The number of items.
     * @return The maximum value that can be put into the knapsack.
     */
    public static int knapSack(int capacity, int[] weights, int[] values, int n) {
        // Basic input validation
        if (n <= 0 || capacity <= 0 || weights == null || values == null || weights.length != n || values.length != n) {
            return 0;
        }

        // dp[w] will be storing the maximum value that can be obtained with knapsack capacity w.
        int[] dp = new int[capacity + 1];

        // Traverse through all items
        for (int i = 0; i < n; i++) {
            // Traverse dp array from right to left to prevent using the same item multiple times
            for (int w = capacity; w >= weights[i]; w--) {
                dp[w] = Math.max(dp[w], values[i] + dp[w - weights[i]]);
            }
        }

        return dp[capacity];
    }

    public static void main(String[] args) {
        // Test Case 1: Standard case
        int[] values1 = {60, 100, 120};
        int[] weights1 = {10, 20, 30};
        int capacity1 = 50;
        System.out.println("Test Case 1: Maximum value is " + knapSack(capacity1, weights1, values1, values1.length));

        // Test Case 2: Small capacity with more items
        int[] values2 = {10, 40, 30, 50};
        int[] weights2 = {5, 4, 6, 3};
        int capacity2 = 10;
        System.out.println("Test Case 2: Maximum value is " + knapSack(capacity2, weights2, values2, values2.length));

        // Test Case 3: All items fit
        int[] values3 = {60, 100, 120};
        int[] weights3 = {10, 20, 30};
        int capacity3 = 100;
        System.out.println("Test Case 3: Maximum value is " + knapSack(capacity3, weights3, values3, values3.length));

        // Test Case 4: No items
        int[] values4 = {};
        int[] weights4 = {};
        int capacity4 = 50;
        System.out.println("Test Case 4: Maximum value is " + knapSack(capacity4, weights4, values4, values4.length));

        // Test Case 5: Zero capacity
        int[] values5 = {60, 100, 120};
        int[] weights5 = {10, 20, 30};
        int capacity5 = 0;
        System.out.println("Test Case 5: Maximum value is " + knapSack(capacity5, weights5, values5, values5.length));
    }
}