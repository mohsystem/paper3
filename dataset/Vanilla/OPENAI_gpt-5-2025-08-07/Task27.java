import java.util.*;

public class Task27 {
    public static int findOutlier(int[] arr) {
        int evenCount = 0;
        int len = arr.length;
        int limit = Math.min(3, len);
        for (int i = 0; i < limit; i++) {
            if ((arr[i] & 1) == 0) evenCount++;
        }
        boolean majorityEven = evenCount >= 2;
        for (int x : arr) {
            if (majorityEven && (x & 1) == 1) return x;
            if (!majorityEven && (x & 1) == 0) return x;
        }
        return 0; // Should not reach here if input meets the problem constraints
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {2, 4, 0, 100, 4, 11, 2602, 36},
            {160, 3, 1719, 19, 11, 13, -21},
            {1, 1, 2},
            {2, 6, 8, 10, 3},
            {-2, -4, -6, -7, -8}
        };
        for (int i = 0; i < tests.length; i++) {
            int outlier = findOutlier(tests[i]);
            System.out.println("Test " + (i + 1) + " outlier: " + outlier);
        }
    }
}