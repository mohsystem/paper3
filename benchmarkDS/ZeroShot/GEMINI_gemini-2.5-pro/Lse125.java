public class Lse125 {

    /**
     * Trims trailing whitespace characters from a string.
     *
     * @param str The string to trim.
     * @return The string with trailing whitespace removed, or the original string if it is null or empty.
     */
    public static String trimTrailingWhitespace(String str) {
        if (str == null || str.isEmpty()) {
            return str;
        }
        int i = str.length() - 1;
        while (i >= 0 && Character.isWhitespace(str.charAt(i))) {
            i--;
        }
        return str.substring(0, i + 1);
    }

    /**
     * Main method to run test cases for the trimTrailingWhitespace function.
     * @param args Command line arguments (not used in this example).
     */
    public static void main(String[] args) {
        String[] testCases = {
            "hello world   ",
            "test\t \t",
            "no_trailing",
            "   \t\n ",
            "  leading and trailing  "
        };

        System.out.println("Running Java test cases:");
        for (String test : testCases) {
            String trimmed = trimTrailingWhitespace(test);
            System.out.printf("Input:  \"%s\"\nOutput: \"%s\"\n\n", test, trimmed);
        }
    }
}