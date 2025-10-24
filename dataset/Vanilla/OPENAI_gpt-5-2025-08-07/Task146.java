import java.util.*;

public class Task146 {
    public static int findMissingNumber(int[] arr) {
        int n = arr.length + 1;
        int xor = 0;
        for (int i = 1; i <= n; i++) xor ^= i;
        for (int v : arr) xor ^= v;
        return xor;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][] {
            new int[] {2, 3, 1, 5},
            new int[] {1},
            new int[] {2},
            new int[] {1, 2, 3, 4, 5, 6, 7, 8, 10},
            new int[] {}
        };
        for (int i = 0; i < tests.length; i++) {
            int missing = findMissingNumber(tests[i]);
            System.out.println("Test " + (i + 1) + " missing number: " + missing);
        }
    }
}