public class Lse19 {

    /**
     * Parses a string to a long, adds 1000, and returns the result.
     * This method is secure against invalid number formats and overflow.
     *
     * @param inputStr The string representation of the number.
     * @return The resulting number after adding 1000.
     * @throws NumberFormatException if the input string is not a valid number
     *         or if the addition would result in an overflow.
     */
    public static long addThousand(String inputStr) throws NumberFormatException {
        if (inputStr == null) {
            throw new NumberFormatException("Input string cannot be null.");
        }
        try {
            long num = Long.parseLong(inputStr.trim());

            // Secure: Check for potential overflow before performing the addition.
            if (num > Long.MAX_VALUE - 1000) {
                throw new NumberFormatException("Input number is too large, addition would cause overflow.");
            }
            return num + 1000;
        } catch (NumberFormatException e) {
            // Re-throw with a more specific message for clarity.
            throw new NumberFormatException("Invalid number format: \"" + inputStr + "\"");
        }
    }

    public static void main(String[] args) {
        // Part 1: Process command-line argument if provided.
        if (args.length == 1) {
            System.out.println("Processing command-line argument: " + args[0]);
            try {
                long result = addThousand(args[0]);
                System.out.println("Result: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println("\n-----------------------------------------\n");
        } else if (args.length > 1) {
             System.err.println("Usage: java Lse19 <number>");
             System.err.println("Ignoring extra arguments and running test cases.");
             System.out.println("\n-----------------------------------------\n");
        }

        // Part 2: Run 5 hardcoded test cases to demonstrate functionality.
        System.out.println("Running 5 test cases...");
        String[] testCases = {
            "50",                           // 1. Valid positive number
            "-50",                          // 2. Valid negative number
            "0",                            // 3. Zero
            "abc",                          // 4. Invalid non-numeric input
            "9223372036854775807"           // 5. Number at Long.MAX_VALUE (will cause overflow)
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": Input = \"" + testCases[i] + "\"");
            try {
                long result = addThousand(testCases[i]);
                System.out.println("  -> Result: " + result);
            } catch (NumberFormatException e) {
                System.err.println("  -> Error: " + e.getMessage());
            }
        }
    }
}