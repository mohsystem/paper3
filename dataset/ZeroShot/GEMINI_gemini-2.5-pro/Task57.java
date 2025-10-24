import java.math.BigInteger;

public class Task57 {

    /**
     * Calculates the factorial of a non-negative integer securely.
     * Uses BigInteger to prevent integer overflow, which can lead to incorrect results.
     *
     * @param n The non-negative integer. Must be >= 0.
     * @return The factorial of n as a BigInteger.
     * @throws IllegalArgumentException if n is negative.
     */
    public static BigInteger calculateFactorial(int n) {
        // 1. Input Validation: Factorial is not defined for negative numbers.
        if (n < 0) {
            throw new IllegalArgumentException("Input must be a non-negative number.");
        }

        // 2. Use BigInteger to handle arbitrarily large results and prevent overflow.
        BigInteger result = BigInteger.ONE;
        
        // 3. Use an iterative approach to avoid stack overflow for large n.
        for (int i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        
        return result;
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testCases = {0, 1, 5, 20, -5};
        
        System.out.println("Java Factorial Test Cases:");
        for (int test : testCases) {
            try {
                BigInteger factorial = calculateFactorial(test);
                System.out.println("Factorial of " + test + " is " + factorial);
            } catch (IllegalArgumentException e) {
                System.out.println("Error for input " + test + ": " + e.getMessage());
            }
        }
    }
}