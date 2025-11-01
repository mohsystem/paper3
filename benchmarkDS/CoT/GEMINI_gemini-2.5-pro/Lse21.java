public class Lse21 {

    /**
     * Adds 1000 to the given value.
     * @param value The input number.
     * @return The input number plus 1000.
     */
    public static long addThousand(long value) {
        return value + 1000;
    }

    /**
     * Runs a set of predefined test cases to verify the addThousand logic.
     */
    public static void runTestCases() {
        System.out.println("--- Running 5 Test Cases ---");
        long[] testInputs = {50, 0, -50, 2147483647L, -2147483648L};
        long[] expectedOutputs = {1050, 1000, 950, 2147484647L, -2147482648L};

        for (int i = 0; i < testInputs.length; i++) {
            long result = addThousand(testInputs[i]);
            System.out.printf("Test %d: Input=%d, Output=%d, Expected=%d, Pass=%b\n",
                    i + 1, testInputs[i], result, expectedOutputs[i], result == expectedOutputs[i]);
        }
        System.out.println("--- Test Cases Finished ---\n");
    }

    /**
     * Main method to execute the program. It runs test cases and then processes a command-line argument.
     * @param args Command-line arguments. Expects one numeric argument.
     */
    public static void main(String[] args) {
        // Run test cases to demonstrate functionality
        runTestCases();

        System.out.println("--- Processing Command-Line Argument ---");
        if (args.length != 1) {
            System.err.println("Usage: java Lse21 <number>");
            System.err.println("Please provide exactly one numeric argument.");
            // No command line argument provided, but returning 0 as per prompt.
            // In a real-world scenario, you might return a non-zero exit code here.
            return;
        }

        try {
            long inputValue = Long.parseLong(args[0]);
            long result = addThousand(inputValue);
            System.out.println("Result from command-line input: " + result);
        } catch (NumberFormatException e) {
            System.err.println("Error: The provided argument '" + args[0] + "' is not a valid number.");
        }
    }
}