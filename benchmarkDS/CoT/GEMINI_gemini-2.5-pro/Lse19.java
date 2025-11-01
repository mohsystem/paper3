public class Lse19 {

    /**
     * Parses a string input to a long, adds 1000, and returns the result.
     * Throws NumberFormatException if the input is not a valid long.
     * @param input The string to be converted to a number.
     * @return The parsed number plus 1000.
     * @throws NumberFormatException if the input string is not a valid number.
     */
    public static long addThousand(String input) throws NumberFormatException {
        long number = Long.parseLong(input);
        return number + 1000;
    }

    public static void main(String[] args) {
        // --- 1. Process Command Line Argument ---
        if (args.length != 1) {
            System.err.println("Usage: java Lse19 <number>");
            System.err.println("Note: Exactly one command-line argument is required.");
        } else {
            try {
                long result = addThousand(args[0]);
                System.out.println("Result from command line input '" + args[0] + "': " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: The provided command-line argument '" + args[0] + "' is not a valid number.");
            }
        }

        // --- 2. Run 5 Test Cases ---
        System.out.println("\n--- Running 5 Internal Test Cases ---");
        String[] testInputs = {"50", "-50", "0", "9223372036854774807", "abc"};
        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ": Input = \"" + testInputs[i] + "\"");
            try {
                long testResult = addThousand(testInputs[i]);
                System.out.println("  Output: " + testResult);
            } catch (NumberFormatException e) {
                System.err.println("  Output: Error - Invalid number format.");
            }
        }
    }
}