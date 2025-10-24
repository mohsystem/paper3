import java.util.stream.Collectors;

public class Task120 {

    /**
     * Escapes special HTML characters in a string to prevent XSS.
     *
     * @param input The raw string to be escaped.
     * @return The HTML-escaped string.
     */
    public static String escapeHtml(String input) {
        if (input == null || input.isEmpty()) {
            return "";
        }
        return input.chars()
            .mapToObj(c -> {
                switch (c) {
                    case '&': return "&amp;";
                    case '<': return "&lt;";
                    case '>': return "&gt;";
                    case '"': return "&quot;";
                    case '\'': return "&#39;"; // or &apos;
                    default: return String.valueOf((char) c);
                }
            })
            .collect(Collectors.joining());
    }

    /**
     * Generates a simple HTML page string displaying user-supplied input securely.
     *
     * @param userInput The user-supplied string.
     * @return A string containing the full HTML page.
     */
    public static String generateWebPage(String userInput) {
        String escapedInput = escapeHtml(userInput);
        return "<!DOCTYPE html>\n" +
               "<html>\n" +
               "<head>\n" +
               "  <title>User Input</title>\n" +
               "</head>\n" +
               "<body>\n" +
               "  <h1>User Comment:</h1>\n" +
               "  <p>" + escapedInput + "</p>\n" +
               "</body>\n" +
               "</html>";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hello, World!",
            "1 < 5",
            "This is a 'quote' & an \"ampersand\"",
            "<script>alert('XSS Attack!')</script>",
            ""
        };

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testCases[i]);
            String webPage = generateWebPage(testCases[i]);
            System.out.println("Generated HTML:\n" + webPage);
        }
    }
}