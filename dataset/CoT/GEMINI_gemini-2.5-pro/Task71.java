import java.util.Optional;

public class Task71 {

    /**
     * Attempts to convert a string to an integer.
     * Handles null, empty, non-numeric, and out-of-range inputs.
     *
     * @param input The string to be converted.
     * @return An Optional<Integer> containing the converted integer if successful, otherwise an empty Optional.
     */
    public static Optional<Integer> convertStringToInt(String input) {
        if (input == null || input.trim().isEmpty()) {
            return Optional.empty();
        }
        try {
            return Optional.of(Integer.parseInt(input.trim()));
        } catch (NumberFormatException e) {
            // This exception catches both invalid formats (e.g., "abc")
            // and values out of the integer range (e.g., "99999999999").
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        // Section 1: Process command-line argument if provided
        if (args.length > 0) {
            System.out.println("--- Processing Command-Line Argument ---");
            String cliInput = args[0];
            System.out.println("Input: \"" + cliInput + "\"");
            Optional<Integer> result = convertStringToInt(cliInput);
            result.ifPresentOrElse(
                value -> System.out.println("Successfully converted to: " + value),
                () -> System.out.println("Error: Conversion failed. Input is not a valid integer.")
            );
            System.out.println("--------------------------------------\n");
        } else {
            System.out.println("--- No Command-Line Argument Provided ---\n");
        }

        // Section 2: Run 5 hardcoded test cases
        System.out.println("--- Running Hardcoded Test Cases ---");
        String[] testCases = {
            "123",          // Valid integer
            "-45",          // Valid negative integer
            "abc",          // Invalid format
            "2147483648",   // Out of range (Integer.MAX_VALUE + 1)
            "  99 "        // Valid with whitespace
        };

        for (int i = 0; i < testCases.length; i++) {
            String testInput = testCases[i];
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: \"" + testInput + "\"");
            Optional<Integer> result = convertStringToInt(testInput);
            result.ifPresentOrElse(
                value -> System.out.println("Successfully converted to: " + value),
                () -> System.out.println("Error: Conversion failed. Input is not a valid integer.")
            );
            System.out.println(); // For spacing
        }
        System.out.println("------------------------------------");
    }
}