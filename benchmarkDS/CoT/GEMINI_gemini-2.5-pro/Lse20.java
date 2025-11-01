public class Lse20 {
    private static final long OFFSET = 1000;

    /**
     * Adds a constant offset to the given value.
     *
     * @param value The input number.
     * @return The value with the offset added.
     */
    public static long addOffset(long value) {
        return value + OFFSET;
    }

    public static void main(String[] args) {
        // We can use the first command-line argument if provided.
        if (args.length == 1) {
            System.out.println("Processing command-line argument: " + args[0]);
            try {
                // Parse the string argument to a long
                long inputValue = Long.parseLong(args[0]);
                long result = addOffset(inputValue);
                System.out.println("Input: " + inputValue + ", Result: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: Input '" + args[0] + "' is not a valid number.");
            }
            System.out.println("\n--- Running Internal Test Cases ---");
        }
        
        // 5 internal test cases for demonstration
        String[] testCases = {
            "10",          // Basic positive number
            "-50",         // Basic negative number
            "0",           // Zero
            "9223372036854774807", // A very large long number (Long.MAX_VALUE - 1000)
            "abc"          // Invalid input
        };

        for (String testCase : testCases) {
            System.out.println("Processing test case: " + testCase);
            try {
                long inputValue = Long.parseLong(testCase);
                long result = addOffset(inputValue);
                System.out.println("Input: " + inputValue + ", Result: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: Input '" + testCase + "' is not a valid number.");
            }
        }
    }
}