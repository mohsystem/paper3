import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse128 {

    private static final int MAX_INPUT_BYTES = 1024;
    private static final int MAX_USERNAME_CHARS = 100;

    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        if (request == null || response == null) {
            throw new IllegalArgumentException("request and response must not be null");
        }

        byte[] buf = new byte[MAX_INPUT_BYTES];
        int read = 0;
        int n;
        while (read < MAX_INPUT_BYTES && (n = request.read(buf, read, MAX_INPUT_BYTES - read)) > 0) {
            read += n;
        }

        String input = new String(buf, 0, Math.max(read, 0), StandardCharsets.UTF_8);
        String username = sanitizeUsername(input);

        String output = "hello " + username;

        byte[] outBytes = output.getBytes(StandardCharsets.UTF_8);
        response.write(outBytes);
        response.flush();

        return output;
    }

    private static String sanitizeUsername(String s) {
        if (s == null) return "anonymous";
        // Remove CR and LF
        s = s.replace("\r", "").replace("\n", "");
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < s.length() && count < MAX_USERNAME_CHARS; i++) {
            char c = s.charAt(i);
            if (isAllowed(c)) {
                sb.append(c);
                count++;
            }
        }
        String out = sb.toString().trim();
        if (out.isEmpty()) out = "anonymous";
        return out;
    }

    private static boolean isAllowed(char c) {
        if (c >= 'a' && c <= 'z') return true;
        if (c >= 'A' && c <= 'Z') return true;
        if (c >= '0' && c <= '9') return true;
        if (c == ' ' || c == '_' || c == '-' || c == '.' || c == '@') return true;
        return false;
    }

    // 5 test cases
    public static void main(String[] args) throws Exception {
        String[] tests = new String[] {
                "Alice",
                "Bob<script>alert(1)</script>",
                "Carol\r\nSet-Cookie: session=evil",
                "Δelta-用户",
                "Dave ../../etc/passwd\t\n"
        };

        for (int i = 0; i < tests.length; i++) {
            ByteArrayInputStream req = new ByteArrayInputStream(tests[i].getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp = new ByteArrayOutputStream();
            String result = http_hello_page(req, resp);
            String written = resp.toString(StandardCharsets.UTF_8);
            System.out.println("Test " + (i + 1) + ": " + result + " | ResponseStream: " + written);
        }
    }
}