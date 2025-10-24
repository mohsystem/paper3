public class Task98 {

    /**
     * Attempts to convert a given string to an integer and prints the outcome.
     *
     * @param input The string to be converted.
     */
    public static void attemptConversion(String input) {
        try {
            // trim() removes leading and trailing whitespace before parsing.
            int number = Integer.parseInt(input.trim());
            System.out.println("Input: \"" + input + "\" -> Success! Converted to integer: " + number);
        } catch (NumberFormatException e) {
            System.out.println("Input: \"" + input + "\" -> Failed. Not a valid integer.");
        }
    }

    public static void main(String[] args) {
        // Array of 5 test cases to demonstrate the functionality.
        String[] testCases = {
            "123",      // Valid positive integer
            "-45",      // Valid negative integer
            "  99  ",   // Valid integer with whitespace
            "abc",      // Invalid string
            "12a34"     // Invalid string with mixed characters
        };

        System.out.println("--- Java: Testing Integer Conversion ---");
        for (String test : testCases) {
            attemptConversion(test);
        }
    }
}