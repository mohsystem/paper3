import java.util.*;

public class Lse126 {
    public static String http_hello_page(String username) {
        String name = (username == null || username.isEmpty()) ? "Guest" : escapeHtml(username);
        StringBuilder sb = new StringBuilder();
        sb.append("<!DOCTYPE html>\n");
        sb.append("<html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body>");
        sb.append("<h1>Hello, ").append(name).append("!</h1>");
        sb.append("<p>Welcome to the hello page.</p>");
        sb.append("</body></html>");
        return sb.toString();
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
        String[] tests = {"Alice", "Bob & Co", "<script>", "", null};
        for (String t : tests) {
            String html = http_hello_page(t);
            System.out.println("=== Test ===");
            System.out.println(html);
        }
    }
}