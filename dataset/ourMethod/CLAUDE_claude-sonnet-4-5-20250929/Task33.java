
import java.util.Arrays;

public class Task33 {
    public static int queueTime(int[] customers, int n) {
        // Input validation
        if (customers == null || n <= 0) {
            return 0;
        }
        
        if (customers.length == 0) {
            return 0;
        }
        
        // Validate all customer times are non-negative
        for (int time : customers) {
            if (time < 0) {
                throw new IllegalArgumentException("Customer time cannot be negative");
            }
        }
        
        // Create array to track time at each till
        int[] tills = new int[Math.min(n, customers.length)];
        
        // Assign each customer to the till with minimum time
        for (int customerTime : customers) {
            // Find till with minimum time
            int minIndex = 0;
            int minTime = tills[0];
            for (int i = 1; i < tills.length; i++) {
                if (tills[i] < minTime) {
                    minTime = tills[i];
                    minIndex = i;
                }
            }
            // Assign customer to that till
            tills[minIndex] += customerTime;
        }
        
        // Find maximum time among all tills
        int maxTime = tills[0];
        for (int i = 1; i < tills.length; i++) {
            if (tills[i] > maxTime) {
                maxTime = tills[i];
            }
        }
        
        return maxTime;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {5, 3, 4};
        System.out.println("Test 1: " + queueTime(test1, 1) + " (expected: 12)");
        
        // Test case 2
        int[] test2 = {10, 2, 3, 3};
        System.out.println("Test 2: " + queueTime(test2, 2) + " (expected: 10)");
        
        // Test case 3
        int[] test3 = {2, 3, 10};
        System.out.println("Test 3: " + queueTime(test3, 2) + " (expected: 12)");
        
        // Test case 4
        int[] test4 = {};
        System.out.println("Test 4: " + queueTime(test4, 1) + " (expected: 0)");
        
        // Test case 5
        int[] test5 = {1, 2, 3, 4, 5};
        System.out.println("Test 5: " + queueTime(test5, 5) + " (expected: 5)");
    }
}
