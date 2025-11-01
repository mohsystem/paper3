public class Lse125 {

    /**
     * Trims trailing whitespace characters from a given string.
     *
     * @param s The string to trim.
     * @return A new string with trailing whitespace removed, or the original string if no trimming is needed.
     */
    public static String trimTrailingWhitespace(String s) {
        if (s == null || s.isEmpty()) {
            return s;
        }

        int len = s.length();
        while (len > 0 && Character.isWhitespace(s.charAt(len - 1))) {
            len--;
        }

        // If length hasn't changed, no trailing whitespace was found.
        // Return original string to avoid creating a new object.
        if (len == s.length()) {
            return s;
        }
        
        return s.substring(0, len);
    }

    public static void main(String[] args) {
        // If command-line arguments are provided, process them.
        if (args.length > 0) {
            System.out.println("Processing command-line arguments:");
            for (String arg : args) {
                String trimmedArg = trimTrailingWhitespace(arg);
                System.out.println("Original: \"" + arg + "\" -> Trimmed: \"" + trimmedArg + "\"");
            }
        } else {
            // Otherwise, run built-in test cases.
            System.out.println("No command-line arguments provided. Running test cases:");
            String[] testCases = {
                "Hello World   ",
                "  leading and trailing  \t\n",
                "NoWhitespace",
                "   ",
                ""
            };

            for (String test : testCases) {
                String trimmedTest = trimTrailingWhitespace(test);
                System.out.println("Original: \"" + test + "\" -> Trimmed: \"" + trimmedTest + "\"");
            }
        }
    }
}