public class Task38 {

    /**
     * Securely escapes user input to prevent Cross-Site Scripting (XSS) and
     * wraps it in a basic HTML structure for display.
     *
     * @param userInput The raw string from the user.
     * @return A string containing a complete HTML page with the sanitized input.
     */
    public static String secureDisplay(String userInput) {
        if (userInput == null) {
            userInput = "";
        }
        StringBuilder sanitizedInput = new StringBuilder();
        for (int i = 0; i < userInput.length(); i++) {
            char c = userInput.charAt(i);
            switch (c) {
                case '<':
                    sanitizedInput.append("&lt;");
                    break;
                case '>':
                    sanitizedInput.append("&gt;");
                    break;
                case '&':
                    sanitizedInput.append("&amp;");
                    break;
                case '"':
                    sanitizedInput.append("&quot;");
                    break;
                case '\'':
                    sanitizedInput.append("&#39;");
                    break;
                default:
                    sanitizedInput.append(c);
                    break;
            }
        }
        return "<html><body><h1>User Input:</h1><p>" + sanitizedInput.toString() + "</p></body></html>";
    }

    public static void main(String[] args) {
        // 5 Test Cases
        String[] testCases = {
            "Hello, World!", // Normal input
            "<script>alert('XSS');</script>", // Malicious script
            "John & Doe > Company", // Input with special HTML characters
            "This is a \"quote\".", // Input with quotes
            "1 < 2 && 3 > 1" // Another input with special characters
        };

        System.out.println("--- Running Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ":");
            System.out.println("Original Input: " + testCases[i]);
            String safeHtml = secureDisplay(testCases[i]);
            System.out.println("Generated Safe HTML: " + safeHtml);
        }
    }
}