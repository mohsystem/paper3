import java.util.Optional;

public class Task71 {

    /**
     * Securely converts a string to an integer.
     * Handles null input, invalid number formats, and integer overflow/underflow.
     *
     * @param input The string to convert.
     * @return An Optional containing the integer if conversion is successful, otherwise an empty Optional.
     */
    public static Optional<Integer> convertStringToInt(String input) {
        if (input == null || input.trim().isEmpty()) {
            System.err.println("Error: Input string is null or empty.");
            return Optional.empty();
        }

        try {
            return Optional.of(Integer.parseInt(input.trim()));
        } catch (NumberFormatException e) {
            // This exception handles both invalid formats (e.g., "abc") and
            // values outside the Integer.MIN_VALUE/MAX_VALUE range.
            System.err.println("Error: '" + input + "' is not a valid integer or is out of range.");
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        // Handle command-line argument if provided
        if (args.length > 0) {
            System.out.println("--- Command-Line Input ---");
            String cliInput = args[0];
            System.out.println("Input: \"" + cliInput + "\"");
            Optional<Integer> result = convertStringToInt(cliInput);
            result.ifPresentOrElse(
                value -> System.out.println("Successfully converted to: " + value),
                () -> System.out.println("Conversion failed.")
            );
            System.out.println("--------------------------\n");
        } else {
            System.out.println("Usage: java Task71 <string_to_convert>");
            System.out.println("Running built-in test cases as no command-line argument was provided.\n");
        }

        // Run 5 test cases
        System.out.println("--- Built-in Test Cases ---");
        String[] testCases = {
            "123",          // 1. Valid positive integer
            "-456",         // 2. Valid negative integer
            "hello",        // 3. Invalid format
            "2147483648",   // 4. Integer overflow
            "-2147483649"   // 5. Integer underflow
        };

        for (String testCase : testCases) {
            System.out.println("Input: \"" + testCase + "\"");
            Optional<Integer> result = convertStringToInt(testCase);
            result.ifPresentOrElse(
                value -> System.out.println("Successfully converted to: " + value),
                () -> System.out.println("Conversion failed.")
            );
            System.out.println(); // for spacing
        }
        System.out.println("---------------------------");
    }
}