import java.util.*;

public class Task33 {
    public static int queueTime(int[] customers, int n) {
        if (customers == null || customers.length == 0) return 0;
        if (n <= 0) throw new IllegalArgumentException("n must be positive");
        PriorityQueue<Integer> tills = new PriorityQueue<>();
        for (int i = 0; i < n; i++) tills.add(0);
        for (int t : customers) {
            int next = tills.poll();
            tills.add(next + t);
        }
        int total = 0;
        while (!tills.isEmpty()) {
            total = Math.max(total, tills.poll());
        }
        return total;
    }

    public static void main(String[] args) {
        int[][] data = new int[][]{
            new int[]{5, 3, 4},
            new int[]{10, 2, 3, 3},
            new int[]{2, 3, 10},
            new int[]{},
            new int[]{1, 2, 3, 4, 5}
        };
        int[] ns = new int[]{1, 2, 2, 1, 100};
        for (int i = 0; i < data.length; i++) {
            int res = queueTime(data[i], ns[i]);
            System.out.println(res);
        }
    }
}