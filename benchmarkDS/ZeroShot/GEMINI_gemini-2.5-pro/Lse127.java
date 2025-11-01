import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

public class Lse127 {

    // Define a reasonable maximum length for a username to prevent resource exhaustion.
    private static final int MAX_USERNAME_LENGTH = 256;

    /**
     * Performs HTML entity encoding to prevent Cross-Site Scripting (XSS).
     *
     * @param input The raw string to encode.
     * @return The HTML-encoded string.
     */
    private static String encodeHTML(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder encoded = new StringBuilder(input.length());
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    encoded.append("&amp;");
                    break;
                case '<':
                    encoded.append("&lt;");
                    break;
                case '>':
                    encoded.append("&gt;");
                    break;
                case '"':
                    encoded.append("&quot;");
                    break;
                case '\'':
                    encoded.append("&#x27;");
                    break;
                case '/':
                    encoded.append("&#x2F;");
                    break;
                default:
                    encoded.append(c);
                    break;
            }
        }
        return encoded.toString();
    }

    /**
     * Reads a username from a request stream, validates it, and writes a secure
     * "Hello [username]" greeting to a response stream.
     *
     * @param request  A Reader representing the client's request stream.
     * @param response A Writer representing the server's response stream.
     */
    public static void http_hello_page(Reader request, Writer response) {
        try (BufferedReader reader = new BufferedReader(request)) {
            // Read the username from the request. Assuming one line of input.
            String username = reader.readLine();

            if (username == null) {
                username = ""; // Treat null input as empty
            }

            // 1. Security: Validate input length.
            if (username.length() > MAX_USERNAME_LENGTH) {
                response.write("Hello! (Username is too long)");
                return;
            }

            // 2. Security: Sanitize output by HTML-encoding the username.
            String sanitizedUsername = encodeHTML(username);

            // Construct and write the response
            response.write("Hello " + sanitizedUsername);

        } catch (IOException e) {
            // In a real application, log this error appropriately.
            try {
                response.write("An error occurred while processing your request.");
            } catch (IOException ignored) {
                // Ignore errors on the error-writing attempt.
            }
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testInputs = {
            // Test Case 1: Normal valid username
            "Alice",
            // Test Case 2: XSS attack attempt
            "<script>alert('xss')</script>",
            // Test Case 3: Input length violation
            "A".repeat(300),
            // Test Case 4: Empty username
            "",
            // Test Case 5: Username with special characters that need encoding
            "Bob&Eve"
        };

        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testInputs[i]);

            try (
                Reader request = new StringReader(testInputs[i]);
                Writer response = new StringWriter();
            ) {
                http_hello_page(request, response);
                System.out.println("Output: " + response.toString());
            } catch (IOException e) {
                System.out.println("Error during test: " + e.getMessage());
            }
            System.out.println();
        }
    }
}