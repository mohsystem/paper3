public class Lse123 {

    /**
     * Trims trailing whitespace characters (space, tab, newline, etc.) from a string.
     * The prompt describes separate loops for whitespace and newlines, but a single
     * loop is more efficient as Character.isWhitespace() handles both.
     *
     * @param str The string to trim.
     * @return The string with trailing whitespace removed.
     */
    public static String trimTrailingWhitespace(String str) {
        if (str == null) {
            return "";
        }
        // Check the argument to make sure that it is a string (handled by type system).

        int i = str.length() - 1;
        
        // The loop trims the trailing whitespace and newlines in the string.
        while (i >= 0 && Character.isWhitespace(str.charAt(i))) {
            i--;
        }

        // The new string is a substring of the original.
        return str.substring(0, i + 1);
    }

    public static void main(String[] args) {
        // The first if statement checks to make sure that the user provided exactly one argument.
        if (args.length != 1) {
            System.err.println("Usage: java Lse123 \"<string to trim>\"");
            System.err.println("Running test cases instead.");

            // Running 5 test cases
            System.out.println("\n--- Running Test Cases ---");

            // Test Case 1: Trailing spaces
            String test1 = "hello world  ";
            System.out.println("Original: \"" + test1 + "\"");
            System.out.println("Trimmed : \"" + trimTrailingWhitespace(test1) + "\"\n");

            // Test Case 2: Trailing mixed whitespace (tabs, newlines)
            String test2 = "test\t\n \r";
            System.out.println("Original: \"" + test2 + "\"");
            System.out.println("Trimmed : \"" + trimTrailingWhitespace(test2) + "\"\n");

            // Test Case 3: No trailing whitespace
            String test3 = "already_clean";
            System.out.println("Original: \"" + test3 + "\"");
            System.out.println("Trimmed : \"" + trimTrailingWhitespace(test3) + "\"\n");

            // Test Case 4: All whitespace
            String test4 = "   \t\n";
            System.out.println("Original: \"" + test4 + "\"");
            System.out.println("Trimmed : \"" + trimTrailingWhitespace(test4) + "\"\n");

            // Test Case 5: Empty string
            String test5 = "";
            System.out.println("Original: \"" + test5 + "\"");
            System.out.println("Trimmed : \"" + trimTrailingWhitespace(test5) + "\"\n");
        } else {
            String input = args[0];
            System.out.println("Original: \"" + input + "\"");
            String trimmed = trimTrailingWhitespace(input);
            System.out.println("Trimmed : \"" + trimmed + "\"");
        }
    }
}