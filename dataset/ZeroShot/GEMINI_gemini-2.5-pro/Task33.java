import java.util.Arrays;

public class Task33 {

    /**
     * Calculates the total time required for all customers to check out.
     *
     * @param customers An array of positive integers representing the time each customer needs.
     * @param n The number of checkout tills.
     * @return The total time required.
     */
    public static long queueTime(int[] customers, int n) {
        if (customers.length == 0) {
            return 0;
        }

        // Use long to prevent potential integer overflow
        long[] tills = new long[n];
        
        // Arrays.fill(tills, 0L); is not necessary as Java initializes arrays to 0 by default.

        for (int customerTime : customers) {
            // Find the till that will be free the soonest
            int minTillIndex = 0;
            for (int i = 1; i < n; i++) {
                if (tills[i] < tills[minTillIndex]) {
                    minTillIndex = i;
                }
            }
            // Assign the next customer to this till
            tills[minTillIndex] += customerTime;
        }

        // The total time is the time the last till becomes free
        long maxTime = 0;
        for (long tillTime : tills) {
            if (tillTime > maxTime) {
                maxTime = tillTime;
            }
        }
        return maxTime;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(queueTime(new int[]{5, 3, 4}, 1)); // Expected: 12

        // Test Case 2
        System.out.println(queueTime(new int[]{10, 2, 3, 3}, 2)); // Expected: 10

        // Test Case 3
        System.out.println(queueTime(new int[]{2, 3, 10}, 2)); // Expected: 12
        
        // Test Case 4 (empty queue)
        System.out.println(queueTime(new int[]{}, 1)); // Expected: 0

        // Test Case 5 (more tills than customers)
        System.out.println(queueTime(new int[]{1, 2, 3, 4, 5}, 100)); // Expected: 5
    }
}