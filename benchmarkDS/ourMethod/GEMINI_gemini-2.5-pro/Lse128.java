import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse128 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_REQUEST_LINES = 10;
    // Regex for a simple, safe username (alphanumeric, underscore, hyphen).
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]+$");

    /**
     * Reads a request stream to find a username, validates it, and writes a
     * "hello [username]" HTTP response to the response stream.
     *
     * @param request  An InputStream containing the client request. The function expects a line
     *                 in the format "username=[value]".
     * @param response An OutputStream to write the HTTP response to.
     * @throws IOException if an I/O error occurs.
     */
    public static void http_hello_page(InputStream request, OutputStream response) throws IOException {
        String username = null;
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(request, StandardCharsets.UTF_8))) {
            String line;
            int linesRead = 0;
            // Limit the number of lines read to prevent a simple DoS attack.
            while ((line = reader.readLine()) != null && linesRead < MAX_REQUEST_LINES) {
                if (line.startsWith("username=")) {
                    username = line.substring("username=".length());
                    break;
                }
                linesRead++;
            }
        }

        // Rule #1: Validate and sanitize input.
        boolean isValid = username != null &&
                          !username.isEmpty() &&
                          username.length() <= MAX_USERNAME_LENGTH &&
                          USERNAME_PATTERN.matcher(username).matches();

        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(response, StandardCharsets.UTF_8))) {
            if (isValid) {
                String body = "hello " + username;
                writer.write("HTTP/1.1 200 OK\r\n");
                writer.write("Content-Type: text/plain; charset=utf-8\r\n");
                writer.write("Content-Length: " + body.getBytes(StandardCharsets.UTF_8).length + "\r\n");
                writer.write("\r\n");
                writer.write(body);
            } else {
                String body = "Bad Request: Invalid or missing username.";
                writer.write("HTTP/1.1 400 Bad Request\r\n");
                writer.write("Content-Type: text/plain; charset=utf-8\r\n");
                writer.write("Content-Length: " + body.getBytes(StandardCharsets.UTF_8).length + "\r\n");
                writer.write("\r\n");
                writer.write(body);
            }
            writer.flush();
        }
    }

    /**
     * Main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Valid ---");
        test("username=Alice\n");

        System.out.println("\n--- Test Case 2: Long Username ---");
        StringBuilder longUsernameBuilder = new StringBuilder();
        for (int i = 0; i < MAX_USERNAME_LENGTH + 1; i++) {
            longUsernameBuilder.append('a');
        }
        test("username=" + longUsernameBuilder.toString() + "\n");

        System.out.println("\n--- Test Case 3: Invalid Characters ---");
        test("username=<script>alert('xss')</script>\n");

        System.out.println("\n--- Test Case 4: No Username ---");
        test("someotherheader=value\n");

        System.out.println("\n--- Test Case 5: Empty Username ---");
        test("username=\n");
    }

    private static void test(String requestPayload) {
        try {
            InputStream requestStream = new ByteArrayInputStream(requestPayload.getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream responseStream = new ByteArrayOutputStream();
            http_hello_page(requestStream, responseStream);
            System.out.println("Request:\n" + requestPayload.trim());
            System.out.println("Response:\n" + responseStream.toString(StandardCharsets.UTF_8));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}