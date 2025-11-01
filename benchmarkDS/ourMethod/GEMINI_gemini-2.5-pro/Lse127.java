import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

public class Lse127 {

    private static final int MAX_USERNAME_LEN = 256;

    /**
     * Escapes characters in a String that are significant in HTML.
     * @param s the String to escape, may be null
     * @return a new escaped String, or an empty string if null input
     */
    private static String escapeHtml(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '<') {
                out.append("&lt;");
            } else if (c == '>') {
                out.append("&gt;");
            } else if (c == '&') {
                out.append("&amp;");
            } else if (c == '"') {
                out.append("&quot;");
            } else if (c == '\'') {
                out.append("&#x27;");
            } else {
                out.append(c);
            }
        }
        return out.toString();
    }

    /**
     * Reads a username from the request, constructs a "Hello [username]" response,
     * and writes it to the response stream.
     * This function reads the username in a bounded way to prevent DoS attacks
     * and sanitizes it to prevent XSS.
     *
     * @param request  The reader stream containing the client request.
     * @param response The writer stream to send the response to.
     * @throws IOException if an I/O error occurs.
     */
    public static void httpHelloPage(Reader request, Writer response) throws IOException {
        char[] buffer = new char[MAX_USERNAME_LEN + 1];
        int charsRead = request.read(buffer, 0, buffer.length);

        String username;
        if (charsRead <= 0) {
            username = "Guest"; // No input
        } else {
            int lineEnd = -1;
            for (int i = 0; i < charsRead; i++) {
                if (buffer[i] == '\r' || buffer[i] == '\n') {
                    lineEnd = i;
                    break;
                }
            }
            
            if (lineEnd != -1) {
                // Newline found, use the part of the buffer before it.
                username = new String(buffer, 0, lineEnd);
            } else { // No newline found in buffer
                if (charsRead > MAX_USERNAME_LEN) {
                    // We read MAX+1 chars and still no newline, so it's too long.
                    username = "Guest"; // Reject long input.
                } else {
                    // End of stream before a newline, but within length limit.
                    username = new String(buffer, 0, charsRead);
                }
            }
        }

        String trimmedUsername = username.trim();
        if (trimmedUsername.isEmpty()) {
            trimmedUsername = "Guest";
        }

        String sanitizedUsername = escapeHtml(trimmedUsername);
        response.write("Hello " + sanitizedUsername);
        response.flush();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testInputs = {
            "Alice",
            "Bob\nnextline",
            "<script>alert('xss')</script>",
            "    ",
            new String(new char[300]).replace('\0', 'A') // Too long username
        };

        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            String input = testInputs[i];
            // To make long string printable
            String printableInput = input.length() > 50 ? input.substring(0, 50) + "..." : input;
            System.out.println("Input: \"" + printableInput.replace("\n", "\\n") + "\"");
            try (Reader request = new StringReader(input);
                 Writer response = new StringWriter()) {
                httpHelloPage(request, response);
                System.out.println("Output: \"" + response.toString() + "\"");
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println();
        }
    }
}