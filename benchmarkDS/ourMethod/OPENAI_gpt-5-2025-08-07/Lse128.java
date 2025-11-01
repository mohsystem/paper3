import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse128 {

    private static final int MAX_INPUT_BYTES = 4096;
    private static final int MAX_USERNAME_LEN = 64;

    public static void http_hello_page(InputStream request, OutputStream response) throws IOException {
        if (request == null || response == null) {
            throw new IllegalArgumentException("Invalid stream");
        }

        // Read up to MAX_INPUT_BYTES safely
        byte[] buf = new byte[512];
        int read;
        int total = 0;
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        BufferedInputStream bis = new BufferedInputStream(request);
        while ((read = bis.read(buf, 0, Math.min(buf.length, MAX_INPUT_BYTES - total))) > 0) {
            baos.write(buf, 0, read);
            total += read;
            if (total >= MAX_INPUT_BYTES) {
                break;
            }
        }

        String input = new String(baos.toByteArray(), StandardCharsets.UTF_8);
        // Use only the first line as username (ignore any extra lines)
        int newlineIdx = indexOfNewline(input);
        String rawUsername = newlineIdx >= 0 ? input.substring(0, newlineIdx) : input;

        String username = sanitizeUsername(rawUsername);
        if (username.isEmpty()) {
            username = "user";
        }

        String output = "hello " + username;
        response.write(output.getBytes(StandardCharsets.UTF_8));
        response.flush();
    }

    private static int indexOfNewline(String s) {
        int r = s.indexOf('\r');
        int n = s.indexOf('\n');
        if (r == -1) return n;
        if (n == -1) return r;
        return Math.min(r, n);
    }

    private static String sanitizeUsername(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(Math.min(s.length(), MAX_USERNAME_LEN));
        int count = 0;
        for (int i = 0; i < s.length() && count < MAX_USERNAME_LEN; i++) {
            char c = s.charAt(i);
            // Allow only a safe subset: letters, digits, space, underscore, hyphen, dot
            if (c >= 0x20 && c <= 0x7E) { // printable ASCII
                if (Character.isLetterOrDigit(c) || c == ' ' || c == '_' || c == '-' || c == '.') {
                    sb.append(c);
                    count++;
                }
            }
        }
        // Trim spaces from ends
        int start = 0;
        int end = sb.length();
        while (start < end && sb.charAt(start) == ' ') start++;
        while (end > start && sb.charAt(end - 1) == ' ') end--;
        return sb.substring(start, end);
    }

    public static void main(String[] args) throws Exception {
        // Test 1: normal
        runTest("alice");
        // Test 2: mix of allowed chars
        runTest("Bob-123");
        // Test 3: injection attempt with CRLF
        runTest("eve\r\nSet-Cookie: x=1");
        // Test 4: very long username (should be truncated)
        runTest("x".repeat(200));
        // Test 5: special unicode and path-like chars
        runTest("张伟/..\\;|$");
    }

    private static void runTest(String username) throws Exception {
        InputStream req = new ByteArrayInputStream(username.getBytes(StandardCharsets.UTF_8));
        ByteArrayOutputStream resp = new ByteArrayOutputStream();
        http_hello_page(req, resp);
        String result = resp.toString(StandardCharsets.UTF_8.name());
        System.out.println(result);
    }
}