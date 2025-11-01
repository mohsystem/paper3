public class Lse123 {

    /**
     * Processes a string by trimming trailing whitespace and removing all newline characters.
     * This implementation adheres to the specific constraints of using loops for these operations.
     *
     * @param inputStr The string to be processed. It is assumed to be a valid string.
     * @return The processed string.
     */
    public static String processString(String inputStr) {
        if (inputStr == null) {
            return null;
        }

        String currentString = inputStr;
        int len = currentString.length();

        // The for loop trims the trailing whitespace in the string.
        int lastNonWhitespaceIndex = -1;
        for (int i = len - 1; i >= 0; i--) {
            if (!Character.isWhitespace(currentString.charAt(i))) {
                lastNonWhitespaceIndex = i;
                break;
            }
        }
        currentString = currentString.substring(0, lastNonWhitespaceIndex + 1);

        // The second for loop trims the newlines.
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < currentString.length(); i++) {
            char c = currentString.charAt(i);
            if (c != '\n' && c != '\r') {
                sb.append(c);
            }
        }
        
        // In Java, strings are not null-terminated in the C-style sense.
        // The JVM manages string representation internally, which is conceptually
        // similar to being "terminated" by its length property.
        return sb.toString();
    }

    public static void main(String[] args) {
        // The prompt implies command-line argument handling, but also requires
        // 5 test cases in main. We will run the test cases directly.
        // For command-line:
        // if (args.length != 1) {
        //     System.out.println("Error: Exactly one argument is required.");
        //     return;
        // }
        
        String[] testCases = {
            "  hello world   \t ",
            "test with\n newlines \n and spaces   ",
            "   \n\t\r  ", // all whitespace
            "no_trailing_whitespace",
            "another\r\ncase with a final newline\n"
        };
        
        System.out.println("Running 5 Java test cases:");
        for (int i = 0; i < testCases.length; i++) {
            String original = testCases[i];
            String processed = processString(original);
            System.out.printf("Test Case %d:\n", i + 1);
            System.out.printf("  Original:  \"%s\"\n", original);
            System.out.printf("  Processed: \"%s\"\n\n", processed);
        }
    }
}