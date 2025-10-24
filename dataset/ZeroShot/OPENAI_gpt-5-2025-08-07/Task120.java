import java.nio.charset.StandardCharsets;

public class Task120 {
    private static String escapeHTML(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                case '/': out.append("&#x2F;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    public static String displayUserInput(String input) {
        String safe = escapeHTML(input);
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\n");
        html.append("<html lang=\"en\">\n<head>\n");
        html.append("<meta charset=\"UTF-8\">\n");
        html.append("<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none';\">\n");
        html.append("<title>Safe Echo</title>\n");
        html.append("</head>\n<body>\n");
        html.append("<h1>Echo</h1>\n");
        html.append("<pre>");
        html.append(safe);
        html.append("</pre>\n");
        html.append("</body>\n</html>\n");
        return html.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello, world!",
            "<script>alert('XSS');</script>",
            "Tom & Jerry > Mickey & Minnie",
            "\"Quotes\" and 'single quotes' and /slashes/",
            "Unicode: Привет, 你好, مرحبا, 😀"
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("----- Test " + (i + 1) + " -----");
            System.out.println(displayUserInput(tests[i]));
        }
    }
}