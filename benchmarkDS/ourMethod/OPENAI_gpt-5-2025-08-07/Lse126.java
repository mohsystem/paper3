import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse126 {
    private static final int MAX_USERNAME_CP = 256;

    // Constructs a safe HTML response for /hello using the given request body (username).
    public static String http_hello_page(String requestBody) {
        String username = requestBody == null ? "" : requestBody;
        // Trim surrounding whitespace
        username = username.trim();
        // Limit to MAX_USERNAME_CP code points to avoid excessive memory usage and DoS
        username = limitCodePoints(username, MAX_USERNAME_CP);
        // HTML-escape to prevent XSS
        String safeUsername = escapeHtml(username);

        StringBuilder outputbuf = new StringBuilder(256 + safeUsername.length());
        outputbuf.append("<!doctype html>")
                 .append("<html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body>")
                 .append("<h1>Hello, ")
                 .append(safeUsername)
                 .append("!</h1>")
                 .append("</body></html>");
        return outputbuf.toString();
    }

    // Helper to limit a string to at most n Unicode code points without cutting surrogate pairs.
    private static String limitCodePoints(String s, int maxCp) {
        if (s == null) return "";
        int cpCount = s.codePointCount(0, s.length());
        if (cpCount <= maxCp) return s;
        int endIndex = s.offsetByCodePoints(0, maxCp);
        return s.substring(0, endIndex);
    }

    // Minimal HTML escaping
    private static String escapeHtml(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); ) {
            int cp = s.codePointAt(i);
            i += Character.charCount(cp);
            switch (cp) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                default:
                    // Avoid control characters except common whitespace
                    if (cp >= 0x20 || cp == '\n' || cp == '\r' || cp == '\t') {
                        sb.appendCodePoint(cp);
                    } else {
                        sb.append(' ');
                    }
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) throws IOException {
        String[] testRequests = new String[] {
            "Alice",
            "",
            "A".repeat(300), // will be truncated to 256 code points
            "<script>alert('x')</script>",
            "Élodie & 张伟"
        };

        for (int i = 0; i < testRequests.length; i++) {
            // Server reads request body (username)
            String username = testRequests[i];

            // Server constructs the response body (HTML)
            String outputbuf = http_hello_page(username);

            // Server writes the response body to the response stream
            ByteArrayOutputStream responseStream = new ByteArrayOutputStream();
            try {
                responseStream.write(outputbuf.getBytes(StandardCharsets.UTF_8));
                // Server closes the response stream
                responseStream.close();
            } finally {
                // Already closed by close(); ensure no resource leak
            }

            // For demonstration: print the response body
            System.out.println("=== Test Case " + (i + 1) + " ===");
            System.out.println(outputbuf);
            System.out.println();
        }
    }
}