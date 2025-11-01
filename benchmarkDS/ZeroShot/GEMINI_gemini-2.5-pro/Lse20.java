public class Lse20 {

    private static final long OFFSET = 1000L;

    /**
     * Adds a constant offset to a given value, checking for overflow.
     *
     * @param value The input number.
     * @return The value with the offset added.
     * @throws ArithmeticException if the addition would cause an overflow.
     */
    public static long addOffset(long value) throws ArithmeticException {
        // Secure: Check for potential overflow before performing the addition.
        // Underflow is not possible when adding a positive constant.
        if (value > Long.MAX_VALUE - OFFSET) {
            throw new ArithmeticException("Overflow detected on addition");
        }
        return value + OFFSET;
    }

    /**
     * Helper method to run and display a single test case.
     * @param testInput The string input to test.
     */
    private static void runTest(String testInput) {
        System.out.print("Test Case: \"" + testInput + "\" -> ");
        try {
            long inputValue = Long.parseLong(testInput);
            long result = addOffset(inputValue);
            System.out.println("Result: " + result);
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid number format.");
        } catch (ArithmeticException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Part 1: Handle actual command line input if provided
        if (args.length == 1) {
            System.out.println("--- Processing Command Line Input ---");
            try {
                long inputValue = Long.parseLong(args[0]);
                long result = addOffset(inputValue);
                System.out.println("Input: " + args[0] + ", Result: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: Input must be a valid 64-bit integer.");
            } catch (ArithmeticException e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println("-------------------------------------\n");
        } else if (args.length > 1) {
            System.err.println("Usage: java Lse20 <single integer value>");
        }

        // Part 2: Run 5+ built-in test cases as requested
        System.out.println("--- Running Built-in Test Cases ---");
        runTest("123");
        runTest("0");
        runTest("-50");
        runTest("9223372036854775807"); // Long.MAX_VALUE, will cause overflow
        runTest("abc"); // Invalid format
        runTest("-9223372036854775808"); // Long.MIN_VALUE
        System.out.println("-----------------------------------");
    }
}