import java.util.Arrays;

public class Task18 {
    // Chain-of-Through Step 1: Problem understanding
    // Convert an array of 0/1 bits (MSB first) into an integer.

    // Chain-of-Through Step 2: Security requirements
    // - Validate input is non-null.
    // - Validate each element is 0 or 1.
    // - Detect and prevent integer overflow.

    // Chain-of-Through Step 3: Secure coding generation
    public static int binaryArrayToInt(int[] bits) {
        if (bits == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        long acc = 0L; // use long to detect overflow before casting to int
        for (int b : bits) {
            if (b != 0 && b != 1) {
                throw new IllegalArgumentException("Array must contain only 0 or 1.");
            }
            acc = (acc << 1) | b;
            if (acc > Integer.MAX_VALUE) {
                throw new ArithmeticException("Integer overflow for the given bit array.");
            }
        }
        return (int) acc;
    }

    // Chain-of-Through Step 4 and 5: Review and finalize (no known vulnerabilities)
    public static void main(String[] args) {
        // 5 test cases
        int[][] tests = new int[][]{
            {0, 0, 0, 1}, // 1
            {0, 0, 1, 0}, // 2
            {0, 1, 0, 1}, // 5
            {1, 0, 0, 1}, // 9
            {1, 1, 1, 1}  // 15
        };
        for (int[] t : tests) {
            int result = binaryArrayToInt(t);
            System.out.println("Testing: " + Arrays.toString(t) + " ==> " + result);
        }
    }
}