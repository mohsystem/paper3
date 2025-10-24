import java.nio.charset.StandardCharsets;

public class Task120 {
    public static String displayOnWeb(String input) {
        String escaped = escapeHtml(input == null ? "" : input);
        StringBuilder html = new StringBuilder();
        html.append("<!doctype html><html><head><meta charset=\"utf-8\"><title>Echo</title></head><body><h1>Echo</h1><pre>");
        html.append(escaped);
        html.append("</pre></body></html>");
        return html.toString();
    }

    private static String escapeHtml(String s) {
        StringBuilder out = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello, world!",
            "<script>alert('xss')</script>",
            "Tom & Jerry",
            "Quotes: \"double\" and 'single'",
            "Line1\nLine2\tTabbed"
        };
        for (String t : tests) {
            System.out.println(displayOnWeb(t));
        }
    }
}