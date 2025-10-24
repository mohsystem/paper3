
import java.util.*;

public class Task161 {
    
    public static int knapsack(int capacity, int[] weights, int[] values) {
        // Input validation
        if (capacity < 0) {
            throw new IllegalArgumentException("Capacity cannot be negative");
        }
        if (weights == null || values == null) {
            throw new IllegalArgumentException("Weights and values arrays cannot be null");
        }
        if (weights.length != values.length) {
            throw new IllegalArgumentException("Weights and values arrays must have the same length");
        }
        if (weights.length == 0) {
            return 0;
        }
        
        // Validate individual weights and values
        for (int i = 0; i < weights.length; i++) {
            if (weights[i] < 0 || values[i] < 0) {
                throw new IllegalArgumentException("Weights and values must be non-negative");
            }
        }
        
        int n = weights.length;
        
        // Prevent integer overflow for large capacity
        if (capacity > Integer.MAX_VALUE / 2) {
            throw new IllegalArgumentException("Capacity is too large");
        }
        
        // Dynamic programming approach
        int[][] dp = new int[n + 1][capacity + 1];
        
        for (int i = 1; i <= n; i++) {
            for (int w = 0; w <= capacity; w++) {
                if (weights[i - 1] <= w) {
                    dp[i][w] = Math.max(
                        values[i - 1] + dp[i - 1][w - weights[i - 1]],
                        dp[i - 1][w]
                    );
                } else {
                    dp[i][w] = dp[i - 1][w];
                }
            }
        }
        
        return dp[n][capacity];
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Basic case
            int[] weights1 = {2, 3, 4, 5};
            int[] values1 = {3, 4, 5, 6};
            int capacity1 = 5;
            System.out.println("Test 1 - Capacity: " + capacity1 + ", Max Value: " + 
                knapsack(capacity1, weights1, values1));
            
            // Test case 2: Single item
            int[] weights2 = {10};
            int[] values2 = {100};
            int capacity2 = 15;
            System.out.println("Test 2 - Capacity: " + capacity2 + ", Max Value: " + 
                knapsack(capacity2, weights2, values2));
            
            // Test case 3: No items fit
            int[] weights3 = {10, 20, 30};
            int[] values3 = {60, 100, 120};
            int capacity3 = 5;
            System.out.println("Test 3 - Capacity: " + capacity3 + ", Max Value: " + 
                knapsack(capacity3, weights3, values3));
            
            // Test case 4: All items fit
            int[] weights4 = {1, 2, 3};
            int[] values4 = {10, 20, 30};
            int capacity4 = 10;
            System.out.println("Test 4 - Capacity: " + capacity4 + ", Max Value: " + 
                knapsack(capacity4, weights4, values4));
            
            // Test case 5: Zero capacity
            int[] weights5 = {5, 10, 15};
            int[] values5 = {50, 100, 150};
            int capacity5 = 0;
            System.out.println("Test 5 - Capacity: " + capacity5 + ", Max Value: " + 
                knapsack(capacity5, weights5, values5));
                
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
