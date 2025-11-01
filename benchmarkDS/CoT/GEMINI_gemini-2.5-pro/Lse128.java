import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse128 {

    /**
     * Sanitizes a string by replacing HTML special characters with their corresponding entities.
     * This is crucial to prevent Cross-Site Scripting (XSS) vulnerabilities.
     *
     * @param str The input string to sanitize.
     * @return The sanitized string safe for HTML embedding.
     */
    private static String escapeHtml(String str) {
        if (str == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : str.toCharArray()) {
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
                    escaped.append("&#39;"); // or &apos;
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Reads a username from a request stream, sanitizes it, and writes a "hello [username]"
     * message to the response stream.
     *
     * @param request  An InputStream containing the client's request. We assume the first line is the username.
     * @param response An OutputStream to write the server's response to.
     */
    public static void http_hello_page(InputStream request, OutputStream response) {
        // Use try-with-resources to ensure streams are closed automatically
        try (
            BufferedReader reader = new BufferedReader(new InputStreamReader(request, StandardCharsets.UTF_8));
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(response, StandardCharsets.UTF_8))
        ) {
            // Read the username from the request. Assume it's the first line.
            String username = reader.readLine();
            if (username == null) {
                username = ""; // Handle empty request
            }

            // Sanitize the username to prevent XSS attacks
            String sanitizedUsername = escapeHtml(username);

            // Construct the response
            String responseBody = "hello " + sanitizedUsername;

            // Write the response
            writer.write(responseBody);
            writer.flush();

        } catch (IOException e) {
            // In a real application, you'd have more robust error handling/logging
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testUsernames = {
            "Alice",                             // Test Case 1: Simple username
            "Bob Smith",                         // Test Case 2: Username with spaces
            "",                                  // Test Case 3: Empty username
            "<script>alert('XSS')</script>",   // Test Case 4: XSS attack attempt
            "User with \"&'<> special chars"   // Test Case 5: Multiple special characters
        };

        String[] expectedOutputs = {
            "hello Alice",
            "hello Bob Smith",
            "hello ",
            "hello &lt;script&gt;alert(&#39;XSS&#39;)&lt;/script&gt;",
            "hello User with &quot;&amp;&#39;&lt;&gt; special chars"
        };

        System.out.println("--- Running Java Tests ---");
        for (int i = 0; i < testUsernames.length; i++) {
            String username = testUsernames[i];
            String expected = expectedOutputs[i];

            // Simulate request and response streams using in-memory byte arrays
            InputStream requestStream = new ByteArrayInputStream(username.getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream responseStream = new ByteArrayOutputStream();

            // Call the function
            http_hello_page(requestStream, responseStream);

            // Get the actual output
            String actual = responseStream.toString(StandardCharsets.UTF_8);

            // Compare and print results
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("  Input:    \"" + username + "\"");
            System.out.println("  Expected: \"" + expected + "\"");
            System.out.println("  Actual:   \"" + actual + "\"");
            System.out.println("  Result:   " + (expected.equals(actual) ? "PASSED" : "FAILED"));
            System.out.println();
        }
    }
}