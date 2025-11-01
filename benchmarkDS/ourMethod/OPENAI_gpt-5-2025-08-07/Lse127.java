import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Lse127 {

    private static final int MAX_INPUT_CHARS = 1024;
    private static final int MAX_USERNAME_CHARS = 64;

    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        if (request == null || response == null) {
            throw new IllegalArgumentException("Invalid stream");
        }

        // Read up to first newline or MAX_INPUT_CHARS
        StringBuilder raw = new StringBuilder();
        try (InputStreamReader reader = new InputStreamReader(request, StandardCharsets.UTF_8)) {
            for (int i = 0; i < MAX_INPUT_CHARS; i++) {
                int ch = reader.read();
                if (ch == -1) {
                    break;
                }
                if (ch == '\n' || ch == '\r') {
                    break;
                }
                raw.append((char) ch);
            }
        }

        // Sanitize: allow [A-Za-z0-9 _.-'], collapse spaces, trim, and limit length
        StringBuilder clean = new StringBuilder();
        boolean lastWasSpace = true; // to trim leading space
        for (int i = 0; i < raw.length() && clean.length() < MAX_USERNAME_CHARS; i++) {
            char c = raw.charAt(i);
            boolean allowed =
                    (c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_' || c == '-' || c == '.' || c == '\'';
            if (allowed) {
                clean.append(c);
                lastWasSpace = false;
            } else if (Character.isWhitespace(c)) {
                if (!lastWasSpace && clean.length() < MAX_USERNAME_CHARS) {
                    clean.append(' ');
                }
                lastWasSpace = true;
            } else {
                // drop disallowed characters
            }
        }
        // trim trailing space
        int len = clean.length();
        if (len > 0 && clean.charAt(len - 1) == ' ') {
            clean.setLength(len - 1);
        }
        String username = clean.length() == 0 ? "user" : clean.toString();
        String responseBody = "Hello " + username;

        // Write response
        byte[] out = responseBody.getBytes(StandardCharsets.UTF_8);
        response.write(out);
        response.flush();

        return responseBody;
    }

    private static void runTest(byte[] requestData) throws IOException {
        ByteArrayInputStream req = new ByteArrayInputStream(requestData);
        ByteArrayOutputStream resp = new ByteArrayOutputStream();
        String result = http_hello_page(req, resp);
        String written = resp.toString(StandardCharsets.UTF_8);
        System.out.println("Returned: " + result);
        System.out.println("Written:  " + written);
        System.out.println("---");
    }

    public static void main(String[] args) throws Exception {
        List<byte[]> tests = new ArrayList<>();

        // 1) Normal name
        tests.add("Alice".getBytes(StandardCharsets.UTF_8));

        // 2) Name with extra spaces and newline
        tests.add("  Bob   Smith  \r\n".getBytes(StandardCharsets.UTF_8));

        // 3) Attempted script injection
        tests.add("Eve<script>alert(1)</script>".getBytes(StandardCharsets.UTF_8));

        // 4) Empty input
        tests.add(new byte[0]);

        // 5) Very long input
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 500; i++) sb.append('X');
        tests.add(sb.toString().getBytes(StandardCharsets.UTF_8));

        for (byte[] t : tests) {
            runTest(t);
        }
    }
}