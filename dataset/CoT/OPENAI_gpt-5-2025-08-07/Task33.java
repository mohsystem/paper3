// Step 1-5 (Chain-of-Through) summarized in comments for clarity without exposing reasoning details:
// 1) Problem: Compute total checkout time using n tills with a single queue.
// 2) Security: Validate inputs minimally, avoid unsafe operations, and handle nulls.
// 3) Implementation: Use a min-heap to assign next customer to the earliest free till.
// 4) Review: Checked for nulls, empty arrays, and positive n.
// 5) Final: Returning integer total time; includes 5 test cases.
//
// Java implementation
import java.util.*;

public class Task33 {
    // Function accepts input as parameters and returns the total time.
    public static int queueTime(int[] customers, int n) {
        if (customers == null || customers.length == 0) return 0;
        if (n <= 0) throw new IllegalArgumentException("Number of tills must be positive.");
        PriorityQueue<Integer> tills = new PriorityQueue<>();
        for (int i = 0; i < n; i++) {
            tills.add(0);
        }
        for (int t : customers) {
            int nextFree = tills.poll();
            int updated = nextFree + t;
            tills.add(updated);
        }
        int total = 0;
        for (int load : tills) {
            if (load > total) total = load;
        }
        return total;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][] {
            new int[]{5, 3, 4},
            new int[]{10, 2, 3, 3},
            new int[]{2, 3, 10},
            new int[]{},
            new int[]{1, 2, 3, 4, 5}
        };
        int[] ns = new int[] {1, 2, 2, 1, 100};
        int[] expected = new int[] {12, 10, 12, 0, 5};

        for (int i = 0; i < tests.length; i++) {
            int res = queueTime(tests[i], ns[i]);
            System.out.println("Test " + (i + 1) + ": " + res + " (expected " + expected[i] + ")");
        }
    }
}