import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

public class Lse127 {

    /**
     * Sanitizes a string to prevent Cross-Site Scripting (XSS) by escaping HTML special characters.
     *
     * @param input The string to sanitize.
     * @return The sanitized string.
     */
    private static String sanitizeHtml(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '<':
                    sb.append("&lt;");
                    break;
                case '>':
                    sb.append("&gt;");
                    break;
                case '&':
                    sb.append("&amp;");
                    break;
                case '"':
                    sb.append("&quot;");
                    break;
                case '\'':
                    sb.append("&#39;");
                    break;
                default:
                    sb.append(c);
                    break;
            }
        }
        return sb.toString();
    }

    /**
     * Reads a username from a request stream, constructs a "Hello [username]" message,
     * and writes it to a response stream. The username is sanitized to prevent XSS.
     *
     * @param request  A Reader (file stream) containing the client request with the username.
     * @param response A Writer (file stream) to send the response to the client.
     */
    public static void http_hello_page(Reader request, Writer response) {
        try (BufferedReader reader = new BufferedReader(request);
             BufferedWriter writer = new BufferedWriter(response)) {

            // Read the username from the request.
            String username = reader.readLine();
            if (username == null) {
                username = "Guest"; // Default username if request is empty
            }

            // Sanitize the username to prevent XSS vulnerabilities.
            String sanitizedUsername = sanitizeHtml(username);

            // Construct the response.
            String helloResponse = "Hello " + sanitizedUsername;

            // Write the response to the response stream.
            writer.write(helloResponse);
            writer.flush();

        } catch (IOException e) {
            // In a real application, proper logging should be implemented.
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testInputs = {
            "Alice",
            "<script>alert('xss')</script>",
            "Bob & Carol",
            "Mr. \"O'Malley\"",
            ""
        };

        String[] expectedOutputs = {
            "Hello Alice",
            "Hello &lt;script&gt;alert('xss')&lt;/script&gt;",
            "Hello Bob &amp; Carol",
            "Hello Mr. &quot;O&#39;Malley&quot;",
            "Hello "
        };

        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: \"" + testInputs[i] + "\"");

            try (StringReader request = new StringReader(testInputs[i]);
                 StringWriter response = new StringWriter()) {

                http_hello_page(request, response);
                String actualOutput = response.toString();

                System.out.println("Expected: \"" + expectedOutputs[i] + "\"");
                System.out.println("Actual  : \"" + actualOutput + "\"");
                System.out.println("Result  : " + (expectedOutputs[i].equals(actualOutput) ? "PASS" : "FAIL"));
            } catch (Exception e) {
                System.out.println("Test failed with an exception: " + e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}