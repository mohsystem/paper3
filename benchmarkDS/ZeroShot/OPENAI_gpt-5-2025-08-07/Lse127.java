import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse127 {
    private static final int MAX_READ_BYTES = 2048;
    private static final int MAX_NAME_CHARS = 100;

    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        if (request == null || response == null) {
            throw new IllegalArgumentException("request/response must not be null");
        }

        String raw = readLimitedUtf8(request, MAX_READ_BYTES);
        String username = sanitizeUsername(raw, MAX_NAME_CHARS);
        if (username.isEmpty()) {
            username = "Guest";
        }
        String message = "Hello " + username;

        byte[] out = message.getBytes(StandardCharsets.UTF_8);
        response.write(out);
        response.flush();
        return message;
    }

    private static String readLimitedUtf8(InputStream in, int maxBytes) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream(Math.min(maxBytes, 4096));
        byte[] buf = new byte[512];
        int remaining = maxBytes;
        while (remaining > 0) {
            int toRead = Math.min(buf.length, remaining);
            int read = in.read(buf, 0, toRead);
            if (read == -1) break;
            bos.write(buf, 0, read);
            remaining -= read;
        }
        return bos.toString(StandardCharsets.UTF_8);
    }

    private static String sanitizeUsername(String input, int maxChars) {
        if (input == null) return "";
        // Remove CR/LF to avoid response splitting
        input = input.replace("\r", "").replace("\n", "");
        // Trim surrounding whitespace
        input = input.trim();

        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < input.length(); ) {
            int cp = input.codePointAt(i);
            i += Character.charCount(cp);

            if (cp == ' ' || cp == '_' || cp == '-' || cp == '.' || cp == '\'') {
                // allowed punctuation/space
            } else if (!Character.isLetterOrDigit(cp)) {
                // disallow other symbols
                continue;
            }
            if (count < maxChars) {
                sb.appendCodePoint(cp);
                count++;
            } else {
                break;
            }
        }
        // Final trim to remove any accidental leading/trailing spaces
        return sb.toString().trim();
    }

    // 5 test cases
    public static void main(String[] args) throws Exception {
        runTest("Alice");
        runTest("  Bob  ");
        runTest("Eve\r\nSet-Cookie: hack");
        runTest("Željko"); // Unicode letters
        runTest("A".repeat(500)); // very long
    }

    private static void runTest(String input) throws Exception {
        ByteArrayInputStream req = new ByteArrayInputStream(input.getBytes(StandardCharsets.UTF_8));
        ByteArrayOutputStream res = new ByteArrayOutputStream();

        String msg = http_hello_page(req, res);

        String written = res.toString(StandardCharsets.UTF_8);
        System.out.println("Returned: " + msg);
        System.out.println("Written : " + written);
        System.out.println("---");
    }
}