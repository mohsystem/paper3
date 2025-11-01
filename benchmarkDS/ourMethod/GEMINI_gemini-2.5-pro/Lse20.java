public class Lse20 {
    private static final int OFFSET = 1000;

    /**
     * Adds a constant offset to an integer value, checking for overflow.
     *
     * @param value The integer to add the offset to.
     * @return The result of the addition.
     * @throws ArithmeticException if the addition would cause an integer overflow.
     */
    public static int addOffset(int value) throws ArithmeticException {
        // Check for potential positive overflow before addition.
        // Adding a positive constant will not cause an underflow.
        if (value > 0 && value > Integer.MAX_VALUE - OFFSET) {
            throw new ArithmeticException("Integer overflow detected.");
        }
        return value + OFFSET;
    }

    /**
     * Main method to read a command-line argument, add an offset, and print the result.
     *
     * @param args Command-line arguments. Expects a single integer value.
     */
    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Usage: java Lse20 <integer_value>");
            System.exit(1);
        }

        try {
            int inputValue = Integer.parseInt(args[0]);
            int result = addOffset(inputValue);
            System.out.println("Result: " + result);
        } catch (NumberFormatException e) {
            System.err.println("Error: Input must be a valid integer.");
            System.exit(1);
        } catch (ArithmeticException e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }

        /*
        To run the test cases, compile the code first:
        javac Lse20.java

        Then run from the command line:

        Test Case 1: Positive number
        Command: java Lse20 50
        Expected Output: Result: 1050

        Test Case 2: Negative number
        Command: java Lse20 -50
        Expected Output: Result: 950

        Test Case 3: Zero
        Command: java Lse20 0
        Expected Output: Result: 1000

        Test Case 4: Invalid input
        Command: java Lse20 abc
        Expected Output: Error: Input must be a valid integer.

        Test Case 5: Overflow
        Command: java Lse20 2147483647
        Expected Output: Error: Integer overflow detected.
        */
    }
}