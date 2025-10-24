import java.nio.charset.StandardCharsets;

public class Task120 {

    // Sanitize and escape user input to prevent XSS, enforce length and character rules
    public static String sanitizeAndEscape(String input, int maxLen) {
        if (input == null) {
            return "";
        }
        StringBuilder out = new StringBuilder(Math.min(input.length(), maxLen) * 2);
        int count = 0;
        for (int i = 0; i < input.length() && count < maxLen; ) {
            int cp = input.codePointAt(i);
            i += Character.charCount(cp);

            boolean allowed = (cp >= 0x20) || cp == 0x09 || cp == 0x0A || cp == 0x0D; // allow printable and basic whitespace
            if (!allowed) {
                continue;
            }

            // Escape only the necessary HTML special characters
            switch (cp) {
                case '&':
                    out.append("&amp;");
                    break;
                case '<':
                    out.append("&lt;");
                    break;
                case '>':
                    out.append("&gt;");
                    break;
                case '"':
                    out.append("&quot;");
                    break;
                case '\'':
                    out.append("&#x27;");
                    break;
                default:
                    out.appendCodePoint(cp);
                    break;
            }
            count++;
        }
        return out.toString();
    }

    // Render a full HTML page embedding the sanitized content
    public static String renderPage(String userInput) {
        final String safe = sanitizeAndEscape(userInput, 200);
        StringBuilder html = new StringBuilder(512 + safe.length());
        html.append("<!DOCTYPE html>")
            .append("<html lang=\"en\">")
            .append("<head>")
            .append("<meta charset=\"UTF-8\"/>")
            .append("<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>")
            .append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>")
            .append("<title>Safe Echo</title>")
            .append("<style>")
            .append("body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,Cantarell,Noto Sans,sans-serif;line-height:1.5;padding:20px;background:#f8f9fa;color:#212529;}")
            .append("pre{background:#fff;border:1px solid #dee2e6;border-radius:8px;padding:12px;white-space:pre-wrap;word-wrap:break-word;}")
            .append(".note{color:#6c757d;font-size:0.9em;margin-top:8px;}")
            .append("</style>")
            .append("</head>")
            .append("<body>")
            .append("<h1>Safe Echo</h1>")
            .append("<p>The content below is sanitized and length-limited to 200 characters.</p>")
            .append("<pre>")
            .append(safe)
            .append("</pre>")
            .append("<p class=\"note\">Try different inputs by modifying the program arguments.</p>")
            .append("</body>")
            .append("</html>");
        return html.toString();
    }

    public static void main(String[] args) {
        // Optional: if a command-line argument is provided, render that one page first
        if (args != null && args.length > 0) {
            String page = renderPage(args[0]);
            System.out.println(page);
        }

        // 5 test cases
        String[] tests = new String[] {
            "Hello, world!",
            "<script>alert('xss')</script>",
            "A".repeat(300),
            "Line1\nLine2 & Line3 <test>",
            "Emoji: 😊 and quotes \" ' & other <tags>"
        };
        for (int i = 0; i < tests.length; i++) {
            String page = renderPage(tests[i]);
            System.out.println(page);
        }
    }
}