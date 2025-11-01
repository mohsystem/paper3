import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse80 {

    public static final int MAX_BUFFER_SIZE = 1024; // bytes; will store at most 1023 chars

    public static final class ScanResult {
        public final String text;
        public final int count;
        public ScanResult(String text, int count) {
            this.text = text;
            this.count = count;
        }
    }

    // Reads from the provided InputStream up to MAX_BUFFER_SIZE-1 bytes or until a newline.
    // The newline (if encountered during reading) is consumed but not stored.
    // Returns the number of bytes stored in the buffer (excluding newline).
    public static int scan(InputStream in, byte[] buffer) throws IOException {
        if (in == null || buffer == null || buffer.length == 0) {
            return 0;
        }
        int limit = Math.max(0, buffer.length - 1); // mimic C-string spare byte
        int count = 0;
        while (count < limit) {
            int b = in.read();
            if (b == -1) {
                break; // EOF
            }
            if (b == '\n') {
                break; // consume newline; do not store
            }
            buffer[count++] = (byte) b;
        }
        // Do not attempt to read further; if limit reached and next is '\n', we leave it for the next call
        return count;
    }

    // Helper that reads using a specific buffer size and returns both text and count.
    public static ScanResult scanToString(InputStream in, int bufferSize) throws IOException {
        if (bufferSize <= 0) bufferSize = 1;
        byte[] buf = new byte[bufferSize];
        int n = scan(in, buf);
        String s = new String(buf, 0, n, StandardCharsets.UTF_8);
        return new ScanResult(s, n);
    }

    private static String visible(String s) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); ++i) {
            char c = s.charAt(i);
            if (c == '\n') sb.append("\\n");
            else if (c == '\r') sb.append("\\r");
            else if (c == '\t') sb.append("\\t");
            else if (c < 0x20 || c == 0x7F) sb.append(String.format("\\x%02X", (int)c));
            else sb.append(c);
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: Simple line with newline
        String t1 = "Hello\nWorld";
        try (InputStream is = new ByteArrayInputStream(t1.getBytes(StandardCharsets.UTF_8))) {
            ScanResult r = scanToString(is, MAX_BUFFER_SIZE);
            System.out.println("T1 count=" + r.count + " text=\"" + visible(r.text) + "\"");
        }

        // Test case 2: Empty line (just newline)
        String t2 = "\n";
        try (InputStream is = new ByteArrayInputStream(t2.getBytes(StandardCharsets.UTF_8))) {
            ScanResult r = scanToString(is, MAX_BUFFER_SIZE);
            System.out.println("T2 count=" + r.count + " text=\"" + visible(r.text) + "\"");
        }

        // Test case 3: Longer than buffer (1200 'A's and newline)
        StringBuilder sb3 = new StringBuilder();
        for (int i = 0; i < 1200; i++) sb3.append('A');
        sb3.append('\n');
        try (InputStream is = new ByteArrayInputStream(sb3.toString().getBytes(StandardCharsets.UTF_8))) {
            ScanResult r = scanToString(is, MAX_BUFFER_SIZE);
            System.out.println("T3 count=" + r.count + " (expected 1023) text_len=" + r.text.length());
        }

        // Test case 4: No newline before EOF
        String t4 = "NoNewlineEOF";
        try (InputStream is = new ByteArrayInputStream(t4.getBytes(StandardCharsets.UTF_8))) {
            ScanResult r = scanToString(is, MAX_BUFFER_SIZE);
            System.out.println("T4 count=" + r.count + " text=\"" + visible(r.text) + "\"");
        }

        // Test case 5: CRLF before next data
        String t5 = "CR\rLF\r\nNext";
        try (InputStream is = new ByteArrayInputStream(t5.getBytes(StandardCharsets.UTF_8))) {
            ScanResult r = scanToString(is, MAX_BUFFER_SIZE);
            System.out.println("T5 count=" + r.count + " text=\"" + visible(r.text) + "\"");
        }
    }
}