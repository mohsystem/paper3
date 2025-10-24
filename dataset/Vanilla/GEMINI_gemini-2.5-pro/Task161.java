public class Task161 {

    public static int knapsack(int capacity, int[] weights, int[] values) {
        int n = values.length;
        if (n == 0) {
            return 0;
        }

        int[] dp = new int[capacity + 1];

        for (int i = 0; i < n; i++) {
            for (int w = capacity; w >= weights[i]; w--) {
                dp[w] = Math.max(dp[w], values[i] + dp[w - weights[i]]);
            }
        }

        return dp[capacity];
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] values1 = {60, 100, 120};
        int[] weights1 = {10, 20, 30};
        int capacity1 = 50;
        System.out.println("Test Case 1: Maximum value = " + knapsack(capacity1, weights1, values1));

        // Test Case 2
        int[] values2 = {10, 20, 30};
        int[] weights2 = {5, 10, 15};
        int capacity2 = 50;
        System.out.println("Test Case 2: Maximum value = " + knapsack(capacity2, weights2, values2));

        // Test Case 3
        int[] values3 = {10, 40, 30, 50};
        int[] weights3 = {5, 4, 6, 3};
        int capacity3 = 10;
        System.out.println("Test Case 3: Maximum value = " + knapsack(capacity3, weights3, values3));
        
        // Test Case 4
        int[] values4 = {};
        int[] weights4 = {};
        int capacity4 = 10;
        System.out.println("Test Case 4: Maximum value = " + knapsack(capacity4, weights4, values4));

        // Test Case 5
        int[] values5 = {60, 100, 120};
        int[] weights5 = {10, 20, 30};
        int capacity5 = 0;
        System.out.println("Test Case 5: Maximum value = " + knapsack(capacity5, weights5, values5));
    }
}