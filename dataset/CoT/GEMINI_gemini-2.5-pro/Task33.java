import java.util.Arrays;

public class Task33 {

    public static int queueTime(int[] customers, int n) {
        if (customers == null || customers.length == 0) {
            return 0;
        }

        int[] tills = new int[n];
        // Arrays are initialized to 0 by default in Java

        for (int customerTime : customers) {
            // Find the till that will be free earliest
            int minTillIndex = 0;
            for (int i = 1; i < n; i++) {
                if (tills[i] < tills[minTillIndex]) {
                    minTillIndex = i;
                }
            }
            // Assign the customer to this till
            tills[minTillIndex] += customerTime;
        }

        // The total time is the time the last customer finishes
        int maxTime = 0;
        for (int tillTime : tills) {
            if (tillTime > maxTime) {
                maxTime = tillTime;
            }
        }
        return maxTime;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(queueTime(new int[]{5, 3, 4}, 1));
        // Test Case 2
        System.out.println(queueTime(new int[]{10, 2, 3, 3}, 2));
        // Test Case 3
        System.out.println(queueTime(new int[]{2, 3, 10}, 2));
        // Test Case 4
        System.out.println(queueTime(new int[]{}, 1));
        // Test Case 5
        System.out.println(queueTime(new int[]{1, 2, 3, 4, 5}, 100));
    }
}