public class Task57 {

    /**
     * Calculates the factorial of a non-negative integer.
     *
     * @param n The non-negative integer. The input is limited to 20
     *          to prevent long overflow.
     * @return The factorial of n as a long.
     * @throws IllegalArgumentException if n is negative.
     * @throws ArithmeticException if n > 20, which would cause an overflow for a long.
     */
    public static long factorial(int n) {
        if (n < 0) {
            throw new IllegalArgumentException("Factorial is not defined for negative numbers.");
        }
        if (n > 20) {
            // 21! overflows a 64-bit long
            throw new ArithmeticException("Input " + n + " is too large, it will cause a long overflow.");
        }
        if (n == 0) {
            return 1L;
        }

        long result = 1L;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    public static void main(String[] args) {
        int[] testCases = {0, 1, 5, 12, 20, -1, 21};

        for (int testCase : testCases) {
            try {
                System.out.println("Factorial of " + testCase + " is: " + factorial(testCase));
            } catch (IllegalArgumentException | ArithmeticException e) {
                System.out.println("Error calculating factorial for " + testCase + ": " + e.getMessage());
            }
        }
    }
}