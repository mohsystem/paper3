import java.util.Arrays;

public class Lse125 {

    /**
     * Trims trailing whitespace characters from a string.
     * Whitespace is defined by Character.isWhitespace().
     *
     * @param s The string to trim.
     * @return The string with trailing whitespace removed, or an empty string if input is null.
     */
    public static String trimTrailingWhitespace(String s) {
        if (s == null) {
            return "";
        }

        int i = s.length() - 1;
        while (i >= 0 && Character.isWhitespace(s.charAt(i))) {
            i--;
        }
        return s.substring(0, i + 1);
    }

    public static void main(String[] args) {
        System.out.println("--- Command Line Argument Processing ---");
        if (args.length > 0) {
            // Process the first command line argument
            String input = args[0];
            String trimmed = trimTrailingWhitespace(input);
            System.out.println("Original: \"" + input + "\"");
            System.out.println("Trimmed:  \"" + trimmed + "\"");
        } else {
            // Provide usage instructions if no argument is given
            System.err.println("Usage: java Lse125 \"<string to trim>\"");
        }

        System.out.println("\n--- Test Cases ---");
        String[] testCases = {
            "hello world   ",
            "test\t\n ",
            "already_trimmed",
            "      ",
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            String testStr = testCases[i];
            String result = trimTrailingWhitespace(testStr);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("  Original: \"" + testStr + "\"");
            System.out.println("  Trimmed:  \"" + result + "\"\n");
        }
    }
}