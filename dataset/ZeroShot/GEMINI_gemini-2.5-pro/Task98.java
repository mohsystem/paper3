import java.util.Optional;

public class Task98 {

    /**
     * Attempts to convert a string to an integer.
     * This method is secure against invalid input by using a try-catch block
     * to handle NumberFormatException, preventing the program from crashing.
     * It also handles null input gracefully.
     *
     * @param input The string to be converted.
     * @return An Optional containing the integer if conversion is successful, otherwise an empty Optional.
     */
    public static Optional<Integer> convertToInteger(String input) {
        if (input == null) {
            return Optional.empty();
        }
        try {
            // trim() removes leading/trailing whitespace before parsing
            int result = Integer.parseInt(input.trim());
            return Optional.of(result);
        } catch (NumberFormatException e) {
            // Catches cases like "abc" or numbers out of Integer's range
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "123",        // Valid positive integer
            "-456",       // Valid negative integer
            "  789  ",    // Valid integer with whitespace
            "abc",        // Invalid non-numeric string
            "2147483648", // Out of range for a 32-bit signed integer
            null          // Null input
        };

        System.out.println("Running Java Test Cases:");
        for (String testCase : testCases) {
            System.out.print("Input: \"" + testCase + "\" -> ");
            Optional<Integer> result = convertToInteger(testCase);
            if (result.isPresent()) {
                System.out.println("Success, value: " + result.get());
            } else {
                System.out.println("Failed to convert.");
            }
        }
    }
}