import java.util.*;

public class Task33 {
    // Calculates total checkout time given customer times and number of tills
    public static int queueTime(int[] customers, int n) {
        if (customers == null || customers.length == 0 || n <= 0) return 0;
        PriorityQueue<Long> pq = new PriorityQueue<>();
        int tills = Math.min(n, customers.length);
        for (int i = 0; i < tills; i++) {
            long t = customers[i];
            if (t < 0) t = 0;
            pq.add(t);
        }
        for (int i = tills; i < customers.length; i++) {
            long t = pq.poll();
            long c = customers[i];
            if (c < 0) c = 0;
            pq.add(t + c);
        }
        long max = 0;
        for (long t : pq) if (t > max) max = t;
        return max > Integer.MAX_VALUE ? Integer.MAX_VALUE : (int) max;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(queueTime(new int[]{5, 3, 4}, 1));      // 12
        System.out.println(queueTime(new int[]{10, 2, 3, 3}, 2));  // 10
        System.out.println(queueTime(new int[]{2, 3, 10}, 2));     // 12
        System.out.println(queueTime(new int[]{}, 1));             // 0
        System.out.println(queueTime(new int[]{1, 2, 3, 4, 5}, 100)); // 5
    }
}