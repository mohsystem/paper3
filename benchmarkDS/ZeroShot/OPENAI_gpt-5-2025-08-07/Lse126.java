import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse126 {

    public static String http_hello_page(String requestBody) {
        String username = sanitizeUsername(requestBody);
        String safeName = escapeHtml(username);

        StringBuilder sb = new StringBuilder(256 + 256);
        sb.append("<!doctype html>\n")
          .append("<html lang=\"en\">\n")
          .append("<head>\n")
          .append("<meta charset=\"utf-8\">\n")
          .append("<title>Hello</title>\n")
          .append("</head>\n")
          .append("<body>\n")
          .append("<h1>Hello, ").append(safeName).append("!</h1>\n")
          .append("</body>\n")
          .append("</html>\n");

        String outputbuf = sb.toString();

        byte[] bytes;
        try (ByteArrayOutputStream responseStream = new ByteArrayOutputStream()) {
            responseStream.write(outputbuf.getBytes(StandardCharsets.UTF_8));
            responseStream.flush();
            bytes = responseStream.toByteArray();
        } catch (IOException e) {
            return outputbuf;
        }

        return new String(bytes, StandardCharsets.UTF_8);
    }

    private static String sanitizeUsername(String username) {
        String u = (username == null) ? "Guest" : username;
        // Limit to 256 code points
        int codePoints = u.codePointCount(0, u.length());
        if (codePoints > 256) {
            int end = u.offsetByCodePoints(0, 256);
            u = u.substring(0, end);
        }
        // Remove ISO control characters
        StringBuilder cleaned = new StringBuilder(u.length());
        u.codePoints().forEach(cp -> {
            if (!Character.isISOControl(cp)) {
                cleaned.appendCodePoint(cp);
            }
        });
        return cleaned.toString();
    }

    private static String escapeHtml(String input) {
        StringBuilder out = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            switch (ch) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                default: out.append(ch);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Alice",
            "<script>alert(1)</script>",
            "Bob_the_Builder_".repeat(30), // very long
            null,
            "Eve\u0007\u0008\n\r\t"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = http_hello_page(tests[i]);
            System.out.println("Test " + (i + 1) + ":\n" + result + "\n");
        }
    }
}