
public class Task33 {
    public static int queueTime(int[] customers, int n) {
        // Input validation
        if (customers == null || customers.length == 0) {
            return 0;
        }
        
        if (n <= 0) {
            return 0;
        }
        
        // If number of tills is greater than or equal to customers, 
        // return the maximum time among all customers
        if (n >= customers.length) {
            int max = 0;
            for (int time : customers) {
                if (time > max) {
                    max = time;
                }
            }
            return max;
        }
        
        // Create an array to represent the tills and their busy time
        int[] tills = new int[n];
        
        // Assign each customer to the till that will be free first
        for (int customerTime : customers) {
            // Find the till with minimum time
            int minIndex = 0;
            int minTime = tills[0];
            for (int i = 1; i < n; i++) {
                if (tills[i] < minTime) {
                    minTime = tills[i];
                    minIndex = i;
                }
            }
            // Assign customer to the till with minimum time
            tills[minIndex] += customerTime;
        }
        
        // Find the maximum time among all tills
        int maxTime = 0;
        for (int time : tills) {
            if (time > maxTime) {
                maxTime = time;
            }
        }
        
        return maxTime;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] customers1 = {5, 3, 4};
        System.out.println("Test 1: " + queueTime(customers1, 1)); // Expected: 12
        
        // Test case 2
        int[] customers2 = {10, 2, 3, 3};
        System.out.println("Test 2: " + queueTime(customers2, 2)); // Expected: 10
        
        // Test case 3
        int[] customers3 = {2, 3, 10};
        System.out.println("Test 3: " + queueTime(customers3, 2)); // Expected: 12
        
        // Test case 4
        int[] customers4 = {};
        System.out.println("Test 4: " + queueTime(customers4, 1)); // Expected: 0
        
        // Test case 5
        int[] customers5 = {1, 2, 3, 4, 5};
        System.out.println("Test 5: " + queueTime(customers5, 5)); // Expected: 5
    }
}
