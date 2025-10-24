import java.util.ArrayList;
import java.util.List;

public class Task104 {

    public static final class BufferResult {
        public final boolean success;
        public final String content;
        public final boolean truncated;
        public final String message;

        public BufferResult(boolean success, String content, boolean truncated, String message) {
            this.success = success;
            this.content = content;
            this.truncated = truncated;
            this.message = message;
        }

        @Override
        public String toString() {
            return "BufferResult{success=" + success + ", content=\"" + content + "\", truncated=" + truncated + ", message=\"" + message + "\"}";
        }
    }

    private static String sanitize(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            if (ch == '\n' || ch == '\t' || (ch >= 32 && ch <= 126)) {
                sb.append(ch);
            }
        }
        return sb.toString();
    }

    public static BufferResult handleInputToFixedBuffer(String input, int bufferSize) {
        if (bufferSize < 1 || bufferSize > 4096) {
            return new BufferResult(false, "", false, "Invalid buffer size");
        }
        String sanitized = sanitize(input);
        int copyLen = Math.min(sanitized.length(), bufferSize);
        char[] buffer = new char[bufferSize];
        // Copy safely with bounds checking
        for (int i = 0; i < copyLen; i++) {
            buffer[i] = sanitized.charAt(i);
        }
        String content = new String(buffer, 0, copyLen);
        boolean truncated = sanitized.length() > copyLen;
        return new BufferResult(true, content, truncated, "OK");
    }

    public static void main(String[] args) {
        int bufSize = 16;
        List<String> tests = new ArrayList<>();
        tests.add("Hello World");                          // fits
        tests.add("");                                     // empty
        tests.add("This is a very long input string!!!");  // long
        tests.add("Hi\u0001\u0002 there");                 // control chars
        tests.add("1234567890ABCDEF");                     // boundary

        for (int i = 0; i < tests.size(); i++) {
            BufferResult res = handleInputToFixedBuffer(tests.get(i), bufSize);
            System.out.println("Test " + (i + 1) + ": " + res.toString());
        }
    }
}