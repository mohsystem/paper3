import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

public class Lse128 {

    /**
     * Escapes characters in a String to be safe for inclusion in HTML.
     * This prevents Cross-Site Scripting (XSS) vulnerabilities.
     * @param input The string to escape.
     * @return The escaped string.
     */
    private static String htmlEscape(String input) {
        if (input == null || input.isEmpty()) {
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
     * Reads a username from a request stream, constructs a secure "hello [username]"
     * HTML response, and writes it to the response stream.
     *
     * @param request  A reader for the request stream, assumed to contain the username.
     * @param response A writer for the response stream.
     * @throws IOException if an I/O error occurs.
     */
    public static void http_hello_page(Reader request, Writer response) throws IOException {
        // In a real application, you would parse a full HTTP request.
        // For this example, we assume the request stream contains only the username on a single line.
        BufferedReader reader = new BufferedReader(request);
        String username = reader.readLine();
        if (username == null) {
            username = "";
        }

        // Securely escape the user-provided username to prevent XSS attacks.
        String escapedUsername = htmlEscape(username);

        String httpResponse = "HTTP/1.1 200 OK\r\n"
                            + "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                            + "<html><body>hello " + escapedUsername + "</body></html>";

        response.write(httpResponse);
        response.flush();
    }

    public static void main(String[] args) {
        String[] testInputs = {
            "Alice",                                  // Test case 1: Normal input
            "Bob Smith",                              // Test case 2: Input with spaces
            "",                                       // Test case 3: Empty input
            "<script>alert('XSS')</script>",          // Test case 4: XSS attack attempt
            "John&Doe\" onload=\"alert('hacked')\""   // Test case 5: XSS attack with quotes and ampersand
        };

        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testInputs[i]);
            try (Reader request = new StringReader(testInputs[i]);
                 Writer response = new StringWriter()) {
                 
                http_hello_page(request, response);
                
                System.out.println("Output:");
                System.out.println(response.toString());
                System.out.println();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}