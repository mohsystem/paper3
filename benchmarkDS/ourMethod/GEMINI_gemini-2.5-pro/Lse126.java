import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;

public class Lse126 {

    private static final int MAX_USERNAME_LENGTH = 128;

    /**
     * Sanitizes a string for inclusion in HTML content by escaping special characters.
     *
     * @param input The string to sanitize.
     * @return The sanitized string.
     */
    public static String htmlEscape(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder(input.length());
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    escaped.append("&amp;");
                    break;
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
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
     * Handles a request for /hello, reading a username from the request stream
     * and writing a personalized HTML page to the response stream.
     *
     * @param requestStream  The input stream containing the request body (username).
     * @param responseStream The output stream to write the HTML response to.
     */
    public static void http_hello_page(InputStream requestStream, OutputStream responseStream) {
        String username = "Guest";
        // The server reads the request body (the username) into the username variable.
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(requestStream, StandardCharsets.UTF_8))) {
            char[] buffer = new char[MAX_USERNAME_LENGTH + 1];
            int bytesRead = reader.read(buffer, 0, MAX_USERNAME_LENGTH);
            if (bytesRead > 0) {
                username = new String(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
            // In a real server, log this error
            System.err.println("Error reading request body: " + e.getMessage());
            username = "Guest"; // Fallback to default
        }

        // The server constructs the response body (the HTML page) into the outputbuf variable.
        String sanitizedUsername = htmlEscape(username);
        String responseBody = String.format("<html><body><h1>Hello, %s!</h1></body></html>", sanitizedUsername);

        // The server writes the response body to the response stream.
        // The server closes the response stream.
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(responseStream, StandardCharsets.UTF_8))) {
            writer.print(responseBody);
            writer.flush();
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Alice",
            "Bob<script>alert(1)</script>",
            "", // Empty username
            "Charles & David",
            "Eve's \"special\" username that is quite long to test the boundaries and see how it behaves with more characters"
        };
        
        int testNum = 1;
        for (String testCase : testCases) {
            System.out.println("--- Test Case " + testNum++ + " ---");
            System.out.println("Input Username: " + testCase);

            try (
                InputStream request = new ByteArrayInputStream(testCase.getBytes(StandardCharsets.UTF_8));
                ByteArrayOutputStream response = new ByteArrayOutputStream()
            ) {
                http_hello_page(request, response);
                String output = response.toString(StandardCharsets.UTF_8);
                System.out.println("Output HTML: " + output);
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println();
        }
    }
}