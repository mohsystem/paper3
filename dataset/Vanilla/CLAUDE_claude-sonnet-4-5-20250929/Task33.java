
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
        
        // Use a min heap to track till finish times
        PriorityQueue<Integer> tills = new PriorityQueue<>();
        
        // Initialize all tills with 0
        for (int i = 0; i < n; i++) {
            tills.offer(0);
        }
        
        // Process each customer
        for (int customerTime : customers) {
            // Get the till that will be free first
            int earliestFree = tills.poll();
            // Add customer time to that till
            tills.offer(earliestFree + customerTime);
        }
        
        // Find the maximum time (last till to finish)
        int maxTime = 0;
        while (!tills.isEmpty()) {
            maxTime = Math.max(maxTime, tills.poll());
        }
        
        return maxTime;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + queueTime(new int[]{5, 3, 4}, 1) + " (Expected: 12)");
        
        // Test case 2
        System.out.println("Test 2: " + queueTime(new int[]{10, 2, 3, 3}, 2) + " (Expected: 10)");
        
        // Test case 3
        System.out.println("Test 3: " + queueTime(new int[]{2, 3, 10}, 2) + " (Expected: 12)");
        
        // Test case 4
        System.out.println("Test 4: " + queueTime(new int[]{1, 2, 3, 4, 5}, 3) + " (Expected: 6)");
        
        // Test case 5
        System.out.println("Test 5: " + queueTime(new int[]{}, 1) + " (Expected: 0)");
    }
}
