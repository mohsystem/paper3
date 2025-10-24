import java.util.Arrays;

public class Task33 {

    /**
     * Calculates the total time required for all customers to check out.
     *
     * @param customers An array of positive integers representing the queue.
     * @param n The number of checkout tills.
     * @return The total time required.
     */
    public static int queueTime(int[] customers, int n) {
        if (customers.length == 0) {
            return 0;
        }

        int[] tills = new int[n];
        // tills are automatically initialized to 0

        for (int customerTime : customers) {
            // Find the till that will be free the soonest
            int minIndex = 0;
            for (int i = 1; i < n; i++) {
                if (tills[i] < tills[minIndex]) {
                    minIndex = i;
                }
            }
            // Assign the customer to this till
            tills[minIndex] += customerTime;
        }

        // The total time is the time the last till becomes free
        int maxTime = 0;
        for (int tillTime : tills) {
            if (tillTime > maxTime) {
                maxTime = tillTime;
            }
        }
        return maxTime;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(queueTime(new int[]{}, 1)); // Expected: 0
        System.out.println(queueTime(new int[]{5, 3, 4}, 1)); // Expected: 12
        System.out.println(queueTime(new int[]{10, 2, 3, 3}, 2)); // Expected: 10
        System.out.println(queueTime(new int[]{2, 3, 10}, 2)); // Expected: 12
        System.out.println(queueTime(new int[]{1, 2, 3, 4, 5}, 100)); // Expected: 5
    }
}