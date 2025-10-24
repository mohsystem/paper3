public class Task98 {

    /**
     * Attempts to convert a string to an integer and prints the result or an error message.
     *
     * @param input The string to be converted.
     */
    public static void convertAndPrint(String input) {
        if (input == null || input.isEmpty()) {
            System.out.println("Error: Invalid input. String is null or empty.");
            return;
        }
        try {
            int number = Integer.parseInt(input);
            System.out.println("Successfully converted to integer: " + number);
        } catch (NumberFormatException e) {
            System.out.println("Error: Cannot convert '" + input + "' to an integer. It is either not a valid number or out of range.");
        }
    }

    public static void main(String[] args) {
        String[] testCases = {"123", "-456", "abc", "2147483648", "123a"};

        System.out.println("--- Running Java Test Cases ---");
        for (String test : testCases) {
            System.out.print("Input: \"" + test + "\" -> ");
            convertAndPrint(test);
        }
        System.out.println("--- End of Test Cases ---\n");
    }
}