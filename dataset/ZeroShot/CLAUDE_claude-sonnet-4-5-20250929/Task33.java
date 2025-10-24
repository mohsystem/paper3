
import java.util.PriorityQueue;

public class Task33 {
    public static int queueTime(int[] customers, int n) {
        if (customers == null || customers.length == 0) {
            return 0;
        }
        
        if (n <= 0) {
            return 0;
        }
        
        if (n >= customers.length) {
            int max = 0;
            for (int time : customers) {
                max = Math.max(max, time);
            }
            return max;
        }
        
        // Use a min heap to track the finish time of each till
        PriorityQueue<Integer> tills = new PriorityQueue<>();
        
        // Initialize all tills with 0
        for (int i = 0; i < n; i++) {
            tills.offer(0);
        }
        
        // Process each customer
        for (int customerTime : customers) {
            // Get the till that will be free first
            int earliestFreeTill = tills.poll();
            // Add the customer to that till
            tills.offer(earliestFreeTill + customerTime);
        }
        
        // The total time is when the last till finishes
        int maxTime = 0;
        while (!tills.isEmpty()) {
            maxTime = Math.max(maxTime, tills.poll());
        }
        
        return maxTime;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {5, 3, 4};
        System.out.println("Test 1: " + queueTime(test1, 1) + " (Expected: 12)");
        
        // Test case 2
        int[] test2 = {10, 2, 3, 3};
        System.out.println("Test 2: " + queueTime(test2, 2) + " (Expected: 10)");
        
        // Test case 3
        int[] test3 = {2, 3, 10};
        System.out.println("Test 3: " + queueTime(test3, 2) + " (Expected: 12)");
        
        // Test case 4
        int[] test4 = {};
        System.out.println("Test 4: " + queueTime(test4, 1) + " (Expected: 0)");
        
        // Test case 5
        int[] test5 = {1, 2, 3, 4, 5};
        System.out.println("Test 5: " + queueTime(test5, 100) + " (Expected: 5)");
    }
}
