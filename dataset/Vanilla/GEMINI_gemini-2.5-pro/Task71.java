public class Task71 {

    /**
     * Attempts to convert a string to an integer and prints the result or an error message.
     * @param s The string to be converted.
     */
    public static void convertStringToInt(String s) {
        try {
            int number = Integer.parseInt(s);
            System.out.println("Input: \"" + s + "\" -> Converted to integer: " + number);
        } catch (NumberFormatException e) {
            System.out.println("Input: \"" + s + "\" -> Error: Cannot convert to integer. " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Handling command-line arguments if provided
        if (args.length > 0) {
            System.out.println("--- Processing Command-Line Arguments ---");
            for (String arg : args) {
                convertStringToInt(arg);
            }
            System.out.println("---------------------------------------");
        }

        // 5 hardcoded test cases
        System.out.println("\n--- Hardcoded Test Cases ---");
        String[] testCases = {"123", "-456", "abc", "99999999999999999999", "789xyz"};
        for (String test : testCases) {
            convertStringToInt(test);
        }
        System.out.println("--------------------------");
    }
}