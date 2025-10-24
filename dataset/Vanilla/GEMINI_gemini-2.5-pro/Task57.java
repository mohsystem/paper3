public class Task57 {

    /**
     * Calculates the factorial of a non-negative integer.
     *
     * @param n The number to calculate the factorial of.
     * @return The factorial of n, or -1 if n is negative.
     */
    public static long factorial(int n) {
        if (n < 0) {
            // Factorial is not defined for negative numbers
            return -1;
        }
        if (n == 0) {
            return 1;
        }
        long result = 1;
        for (int i = 1; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    public static void main(String[] args) {
        System.out.println("Java Factorial Test Cases:");
        int[] testCases = {0, 1, 5, 10, 15};
        for (int test : testCases) {
            System.out.println("Factorial of " + test + " is " + factorial(test));
        }
    }
}