public class Lse123 {

    /**
     * Processes a string by trimming trailing whitespace and newlines in two separate steps.
     * @param input The string to process.
     * @return The processed string.
     */
    public static String processString(String input) {
        if (input == null) {
            return null;
        }

        // The for loop trims the trailing whitespace (space, tab) in the string.
        int len = input.length();
        int i = len - 1;
        for (; i >= 0; i--) {
            char c = input.charAt(i);
            if (c != ' ' && c != '\t') {
                break;
            }
        }
        String trimmedWhitespace = input.substring(0, i + 1);

        // The second for loop trims the newlines.
        len = trimmedWhitespace.length();
        int j = len - 1;
        for (; j >= 0; j--) {
            char c = trimmedWhitespace.charAt(j);
            if (c != '\n' && c != '\r') {
                break;
            }
        }
        
        // The last line null terminates the string (by creating the final substring).
        return trimmedWhitespace.substring(0, j + 1);
    }

    public static void main(String[] args) {
        // The first if statement checks to make sure that the user provided exactly one argument.
        // We simulate this with test cases as this is a single file.
        if (args.length == 1) {
            // Check the argument to make sure that it is a string (guaranteed in Java).
            String arg = args[0];
            System.out.println("Processing command-line argument:");
            System.out.println("Original: \"" + arg + "\"");
            String result = processString(arg);
            System.out.println("Processed: \"" + result + "\"");
            System.out.println("---------------------------------");
        } else {
             System.out.println("Incorrect argument count. Running test cases instead.");
             System.out.println("---------------------------------");
        }
        
        String[] testCases = {
            "  hello world   \t\n\r",
            "test case 2\t\t",
            "no trailing whitespace",
            "only newlines\n\n\r\n",
            "   \t \n \r "
        };

        for (int k = 0; k < testCases.length; k++) {
            String test = testCases[k];
            System.out.println("Test Case " + (k + 1) + ":");
            // A helper lambda to print strings with visible special characters
            java.util.function.Function<String, String> escape = s -> 
                s.replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t");

            System.out.println("Original:  \"" + escape.apply(test) + "\"");
            String processed = processString(test);
            System.out.println("Processed: \"" + escape.apply(processed) + "\"");
            System.out.println();
        }
    }
}