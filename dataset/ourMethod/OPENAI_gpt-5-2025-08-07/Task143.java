import java.util.Arrays;

public class Task143 {
    private static final int MAX_LEN = 1_000_000;

    public static int[] sortAscending(int[] input) {
        if (input == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        if (input.length > MAX_LEN) {
            throw new IllegalArgumentException("Input array too large.");
        }
        int[] copy = Arrays.copyOf(input, input.length);
        Arrays.sort(copy);
        return copy;
    }

    private static void runTest(int testNumber, int[] input) {
        try {
            int[] sorted = sortAscending(input);
            System.out.println("Test " + testNumber + " input:  " + Arrays.toString(input));
            System.out.println("Test " + testNumber + " output: " + Arrays.toString(sorted));
        } catch (IllegalArgumentException ex) {
            System.out.println("Test " + testNumber + " error: " + ex.getMessage());
        }
        System.out.println("----");
    }

    public static void main(String[] args) {
        int[] t1 = new int[]{3, 1, 2};
        int[] t2 = new int[]{-5, -10, 0, 5};
        int[] t3 = new int[]{};
        int[] t4 = new int[]{1, 1, 1, 1};
        int[] t5 = new int[]{Integer.MAX_VALUE, Integer.MIN_VALUE, 0, -1, 1};

        runTest(1, t1);
        runTest(2, t2);
        runTest(3, t3);
        runTest(4, t4);
        runTest(5, t5);
    }
}