import java.util.Arrays;
import java.util.PriorityQueue;

public class Task33 {

    public static int queueTime(int[] customers, int n) {
        if (customers == null) {
            throw new IllegalArgumentException("customers cannot be null");
        }
        if (n <= 0) {
            throw new IllegalArgumentException("n must be positive");
        }
        for (int t : customers) {
            if (t < 0) {
                throw new IllegalArgumentException("customer times must be non-negative");
            }
        }
        if (customers.length == 0) {
            return 0;
        }
        int k = Math.min(n, customers.length);
        PriorityQueue<Integer> tills = new PriorityQueue<>(k);
        for (int i = 0; i < k; i++) {
            tills.add(customers[i]);
        }
        for (int i = k; i < customers.length; i++) {
            int min = tills.poll();
            tills.add(min + customers[i]);
        }
        int max = 0;
        while (!tills.isEmpty()) {
            int val = tills.poll();
            if (val > max) {
                max = val;
            }
        }
        return max;
    }

    public static void main(String[] args) {
        int[][] testCustomers = {
            {5, 3, 4},
            {10, 2, 3, 3},
            {2, 3, 10},
            {},
            {1, 2, 3, 4, 5}
        };
        int[] testN = {1, 2, 2, 1, 100};
        for (int i = 0; i < testCustomers.length; i++) {
            int result = queueTime(testCustomers[i], testN[i]);
            System.out.println("Test " + (i + 1) + " result: " + result);
        }
    }
}