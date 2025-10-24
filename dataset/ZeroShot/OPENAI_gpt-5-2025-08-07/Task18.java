import java.util.Arrays;

public class Task18 {
    public static long binaryArrayToInt(int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        long acc = 0L;
        for (int i = 0; i < arr.length; i++) {
            int bit = arr[i];
            if (bit != 0 && bit != 1) {
                throw new IllegalArgumentException("Array must contain only 0 or 1 at index " + i);
            }
            if (acc > (Long.MAX_VALUE - bit) / 2L) {
                throw new ArithmeticException("Overflow computing value for provided binary array");
            }
            acc = (acc << 1) + bit;
        }
        return acc;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {0, 0, 0, 1}, // 1
            {0, 0, 1, 0}, // 2
            {0, 1, 0, 1}, // 5
            {1, 0, 0, 1}, // 9
            {1, 1, 1, 1}  // 15
        };
        for (int[] t : tests) {
            try {
                long result = binaryArrayToInt(t);
                System.out.println("Testing: " + Arrays.toString(t) + " ==> " + result);
            } catch (RuntimeException ex) {
                System.out.println("Error for " + Arrays.toString(t) + ": " + ex.getMessage());
            }
        }
    }
}