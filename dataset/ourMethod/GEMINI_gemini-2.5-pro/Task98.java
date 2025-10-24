import java.util.Scanner;

public class Task98 {

    /**
     * Attempts to convert a string to an integer and prints the result or an error message.
     * This function handles null input, invalid number formats, and values outside the
     * integer range by catching NumberFormatException. It also trims leading/trailing whitespace.
     *
     * @param input The string to be converted to an integer.
     */
    public static void convertStringToInteger(String input) {
        if (input == null) {
            System.out.println("Input: null -> Error: Input string cannot be null.");
            return;
        }

        try {
            // trim() removes leading and trailing whitespace
            int result = Integer.parseInt(input.trim());
            System.out.println("Input: \"" + input + "\" -> Success: " + result);
        } catch (NumberFormatException e) {
            System.out.println("Input: \"" + input + "\" -> Error: Not a valid integer or it is out of range.");
        }
    }

    /**
     * Main method to run test cases for the string-to-integer conversion.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        String[] testCases = {
            "123",              // Valid integer
            "  -456  ",         // Valid integer with whitespace
            "abc",              // Invalid characters
            "2147483648",       // Out of range (Integer.MAX_VALUE + 1)
            "98.7"              // Not an integer
        };

        System.out.println("--- Running 5 Test Cases ---");
        for (String test : testCases) {
            convertStringToInteger(test);
        }
        System.out.println("--- Test Cases Finished ---");
    }
}