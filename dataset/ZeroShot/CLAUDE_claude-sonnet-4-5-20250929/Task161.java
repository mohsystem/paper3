
import java.util.*;

public class Task161 {
    public static int knapsack(int capacity, int[] weights, int[] values) {
        if (capacity <= 0 || weights == null || values == null || 
            weights.length == 0 || weights.length != values.length) {
            return 0;
        }
        
        int n = weights.length;
        int[][] dp = new int[n + 1][capacity + 1];
        
        for (int i = 1; i <= n; i++) {
            for (int w = 1; w <= capacity; w++) {
                if (weights[i - 1] <= w) {
                    dp[i][w] = Math.max(dp[i - 1][w], 
                                       values[i - 1] + dp[i - 1][w - weights[i - 1]]);
                } else {
                    dp[i][w] = dp[i - 1][w];
                }
            }
        }
        
        return dp[n][capacity];
    }
    
    public static void main(String[] args) {
        // Test case 1
        int capacity1 = 50;
        int[] weights1 = {10, 20, 30};
        int[] values1 = {60, 100, 120};
        System.out.println("Test 1 - Capacity: " + capacity1 + ", Max Value: " + 
                          knapsack(capacity1, weights1, values1));
        
        // Test case 2
        int capacity2 = 10;
        int[] weights2 = {5, 4, 6, 3};
        int[] values2 = {10, 40, 30, 50};
        System.out.println("Test 2 - Capacity: " + capacity2 + ", Max Value: " + 
                          knapsack(capacity2, weights2, values2));
        
        // Test case 3
        int capacity3 = 7;
        int[] weights3 = {1, 3, 4, 5};
        int[] values3 = {1, 4, 5, 7};
        System.out.println("Test 3 - Capacity: " + capacity3 + ", Max Value: " + 
                          knapsack(capacity3, weights3, values3));
        
        // Test case 4
        int capacity4 = 15;
        int[] weights4 = {2, 3, 5, 7, 1};
        int[] values4 = {10, 5, 15, 7, 6};
        System.out.println("Test 4 - Capacity: " + capacity4 + ", Max Value: " + 
                          knapsack(capacity4, weights4, values4));
        
        // Test case 5
        int capacity5 = 8;
        int[] weights5 = {2, 3, 4, 5};
        int[] values5 = {3, 4, 5, 6};
        System.out.println("Test 5 - Capacity: " + capacity5 + ", Max Value: " + 
                          knapsack(capacity5, weights5, values5));
    }
}
