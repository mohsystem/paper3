public class Lse21 {

    /**
     * Parses a string input to a long, adds 1000, and checks for overflow.
     *
     * @param input The string representation of the number.
     * @return The number after adding 1000.
     * @throws NumberFormatException if the input is not a valid long.
     * @throws ArithmeticException if adding 1000 would cause an overflow.
     */
    public static long processValue(String input) throws NumberFormatException, ArithmeticException {
        long value;
        try {
            value = Long.parseLong(input);
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Input must be a valid integer.");
        }

        if (value > Long.MAX_VALUE - 1000) {
            throw new ArithmeticException("Input value is too large, adding 1000 would cause an overflow.");
        }

        return value + 1000;
    }

    /**
     * Runs a set of test cases against the processValue method.
     */
    public static void runTests() {
        System.out.println("--- Running Test Cases ---");
        String[] testInputs = {
            "10",                     // Valid positive
            "0",                      // Valid zero
            "-50",                    // Valid negative
            "not a number",           // Invalid format
            String.valueOf(Long.MAX_VALUE) // Overflow on add
        };
        
        for (String input : testInputs) {
            System.out.print("Input: \"" + input + "\" -> ");
            try {
                long result = processValue(input);
                System.out.println("Output: " + result);
            } catch (NumberFormatException | ArithmeticException e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }

    /**
     * Main method. If one command-line argument is provided, it processes it.
     * If no arguments are provided, it runs test cases.
     * Otherwise, it shows usage information.
     *
     * @param args Command-line arguments. Expects one integer value.
     */
    public static void main(String[] args) {
        if (args.length == 1) {
            try {
                long result = processValue(args[0]);
                System.out.println(result);
                // Return 0 (success) is implicit
            } catch (NumberFormatException | ArithmeticException e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } else if (args.length == 0) {
            runTests();
        } else {
            System.err.println("Usage: java Lse21 <integer>");
            System.exit(1);
        }
    }
}