import java.util.OptionalLong;

public class Lse19 {

    /**
     * Parses a string to a long, adds 1000 to it, and returns the result.
     * This method is secure against NumberFormatException and integer overflow.
     *
     * @param inputStr The string to parse.
     * @return An OptionalLong containing the result if successful, otherwise an empty OptionalLong.
     */
    public static OptionalLong addThousand(String inputStr) {
        if (inputStr == null) {
            return OptionalLong.empty();
        }
        try {
            long value = Long.parseLong(inputStr);
            // Check for potential overflow before adding
            if (value > Long.MAX_VALUE - 1000) {
                System.err.println("Error: Input value " + value + " is too large and would cause an overflow.");
                return OptionalLong.empty();
            }
            return OptionalLong.of(value + 1000);
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid input '" + inputStr + "'. Please provide a valid integer.");
            return OptionalLong.empty();
        }
    }

    /**
     * Runs a series of test cases for the addThousand method.
     */
    public static void runTests() {
        System.out.println("\n--- Running Test Cases ---");
        String[] testCases = {"10", "0", "-50", "9223372036854775807", "not a number", "9223372036854774807"};

        for (String test : testCases) {
            System.out.print("Test case: \"" + test + "\" -> ");
            OptionalLong result = addThousand(test);
            if (result.isPresent()) {
                System.out.println("Result: " + result.getAsLong());
            } else {
                System.out.println("Handled error as expected.");
            }
        }
        System.out.println("--- Test Cases Finished ---\n");
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            System.out.println("Processing command line argument: " + args[0]);
            OptionalLong result = addThousand(args[0]);
            if (result.isPresent()) {
                System.out.println("Result: " + result.getAsLong());
            } else {
                System.out.println("Failed to process command line argument.");
            }
        } else {
            System.out.println("Usage: java Lse19 <number>");
            System.out.println("No command line argument provided. Running test cases instead.");
        }
        
        // Running 5+ test cases as requested.
        runTests();
    }
}