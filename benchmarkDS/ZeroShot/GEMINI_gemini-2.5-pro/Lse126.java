public class Lse126 {

    /**
     * Sanitizes a string by escaping characters that have special meaning in HTML.
     * This is a crucial security measure to prevent Cross-Site Scripting (XSS) attacks.
     *
     * @param input The string to be sanitized.
     * @return The sanitized HTML-safe string.
     */
    private static String escapeHtml(String input) {
        if (input == null || input.isEmpty()) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Simulates handling an HTTP request for /hello.
     * It takes a username, securely escapes it to prevent XSS,
     * and constructs an HTML response.
     *
     * @param username The username read from the request body.
     * @return A string containing the full HTML page response.
     */
    public static String http_hello_page(String username) {
        // Securely escape the username to prevent XSS vulnerabilities.
        String sanitizedUsername = escapeHtml(username);

        // Construct the response body (the HTML page).
        String outputbuf = "<html><body><h1>Hello, " + sanitizedUsername + "!</h1></body></html>";

        // In a real server, this would be written to the response stream.
        // Here, we just return it.
        return outputbuf;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Alice",
            "Bob Smith",
            "",
            "<script>alert('XSS Attack!')</script>",
            "O'Malley & Sons"
        };

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:  " + testCases[i]);
            String response = http_hello_page(testCases[i]);
            System.out.println("Output: " + response);
            System.out.println();
        }
    }
}