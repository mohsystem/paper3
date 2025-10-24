import java.util.stream.Stream;

public class Task120 {

    /**
     * Sanitizes user input by escaping HTML special characters to prevent XSS attacks,
     * and then embeds it into a simple HTML page structure.
     *
     * @param userInput The raw string provided by the user.
     * @return A string containing a full HTML document with the sanitized user input.
     */
    public static String generateSafeHtml(String userInput) {
        if (userInput == null || userInput.isEmpty()) {
            return "<html><body><p>User input: </p></body></html>";
        }

        StringBuilder sanitizedInput = new StringBuilder();
        for (char c : userInput.toCharArray()) {
            switch (c) {
                case '&':
                    sanitizedInput.append("&amp;");
                    break;
                case '<':
                    sanitizedInput.append("&lt;");
                    break;
                case '>':
                    sanitizedInput.append("&gt;");
                    break;
                case '"':
                    sanitizedInput.append("&quot;");
                    break;
                case '\'':
                    sanitizedInput.append("&#39;"); // &apos; is not supported in HTML4
                    break;
                default:
                    sanitizedInput.append(c);
                    break;
            }
        }

        return "<html><body><p>User input: " + sanitizedInput.toString() + "</p></body></html>";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hello, World!",
            "1 < 5 is true",
            "He said, \"It's a & b > c\"",
            "<script>alert('XSS attack!');</script>",
            ""
        };

        System.out.println("--- Java Test Cases ---");
        for (String testCase : testCases) {
            System.out.println("Input: " + testCase);
            String safeHtml = generateSafeHtml(testCase);
            System.out.println("Output: " + safeHtml);
            System.out.println();
        }
    }
}