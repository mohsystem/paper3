import java.util.Arrays;

public final class Task26 {

    // Finds the one integer that appears an odd number of times using XOR.
    public static int findOdd(int[] arr) {
        if (arr == null || arr.length == 0) {
            throw new IllegalArgumentException("Input array must not be null or empty.");
        }
        int result = 0;
        for (int v : arr) {
            result ^= v;
        }
        return result;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
                {7},
                {0},
                {1, 1, 2},
                {0, 1, 0, 1, 0},
                {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
        };

        for (int i = 0; i < tests.length; i++) {
            try {
                int res = findOdd(tests[i]);
                System.out.println("Test " + (i + 1) + " input=" + Arrays.toString(tests[i]) + " result=" + res);
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i + 1) + " error: " + ex.getMessage());
            }
        }
    }
}