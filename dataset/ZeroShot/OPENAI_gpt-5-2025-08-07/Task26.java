import java.util.Arrays;

public class Task26 {
    public static int findOdd(int[] arr) {
        if (arr == null || arr.length == 0) {
            throw new IllegalArgumentException("Input array must not be null or empty");
        }
        int x = 0;
        for (int v : arr) {
            x ^= v;
        }
        return x;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{7},
            new int[]{0},
            new int[]{1, 1, 2},
            new int[]{0, 1, 0, 1, 0},
            new int[]{1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
        };

        for (int i = 0; i < tests.length; i++) {
            try {
                int res = findOdd(tests[i]);
                System.out.println("Test " + (i + 1) + " result: " + res);
            } catch (IllegalArgumentException e) {
                System.out.println("Test " + (i + 1) + " error: " + e.getMessage());
            }
        }
    }
}