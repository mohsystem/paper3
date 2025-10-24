import java.util.InputMismatchException;

public class Task57 {

    /**
     * Calculates the factorial of a non-negative integer.
     * The input is restricted to the range [0, 20] to prevent long overflow.
     *
     * @param n The non-negative integer.
     * @return The factorial of n.
     * @throws IllegalArgumentException if n is negative or greater than 20.
     */
    public static long factorial(int n) {
        // Rule #4: Validate input range to prevent invalid operations and overflow.
        if (n < 0 || n > 20) {
            throw new IllegalArgumentException("Input must be an integer between 0 and 20.");
        }
        
        long result = 1L;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    public static void main(String[] args) {
        int[] testCases = {0, 1, 5, 10, -1, 21}; // 5+ test cases including edge/error cases

        System.out.println("Running test cases for factorial calculation:");
        for (int testValue : testCases) {
            System.out.println("-------------------------------------");
            System.out.println("Calculating factorial for: " + testValue);
            try {
                // Rule #8: Ensure all exceptions are caught and handled.
                long result = factorial(testValue);
                System.out.println("Result: " + result);
            } catch (IllegalArgumentException e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
        System.out.println("-------------------------------------");
    }
}