import java.util.*;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Lse150 {
    // Chain-of-Through Steps (high-level, non-sensitive):
    // 1) Problem understanding: render main page with a form; handle post() to insert message then redirect.
    // 2) Security requirements: validate inputs, bound lengths, escape on output, avoid code injection.
    // 3) Secure coding generation: parameterized-like logic with strict validation and HTML escaping.
    // 4) Code review: ensure no dangerous concatenation without escaping; limit inputs; safe collections.
    // 5) Secure code output: finalized with mitigations and tests.

    private static final List<Message> MESSAGES = new ArrayList<>();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_\\-]{1,32}$");
    private static final int MAX_MESSAGE_LEN = 500;
    private static final int MAX_MESSAGES = 1000;

    private static class Message {
        final String user;
        final String text;
        Message(String u, String t) { this.user = u; this.text = t; }
    }

    public static String post(String username, String message) {
        // Simulated request handler: returns a redirect string
        boolean ok = insert_user_message_in_db(username, message);
        if (ok) return "REDIRECT: /";
        return "REDIRECT: /?error=invalid";
    }

    public static boolean insert_user_message_in_db(String username, String message) {
        String u = sanitizeUsername(username);
        String m = sanitizeMessage(message);
        if (u == null || m == null) return false;
        if (MESSAGES.size() >= MAX_MESSAGES) return false;
        MESSAGES.add(new Message(u, m));
        return true;
    }

    public static String mainPage() {
        StringBuilder sb = new StringBuilder();
        sb.append("<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>");
        sb.append("<h1>Post a message</h1>");
        sb.append("<form method=\"POST\" action=\"/post\">");
        sb.append("<label>Username: <input type=\"text\" name=\"username\" maxlength=\"32\" required></label><br>");
        sb.append("<label>Message: <textarea name=\"message\" maxlength=\"").append(MAX_MESSAGE_LEN).append("\" required></textarea></label><br>");
        sb.append("<button type=\"submit\">Send</button>");
        sb.append("</form>");
        sb.append("<h2>Recent Messages</h2><ul>");
        for (Message msg : MESSAGES) {
            sb.append("<li><strong>")
              .append(htmlEscape(msg.user))
              .append("</strong>: ")
              .append(htmlEscape(msg.text))
              .append("</li>");
        }
        sb.append("</ul></body></html>");
        return sb.toString();
    }

    private static String sanitizeUsername(String username) {
        if (username == null) return null;
        String u = username.trim();
        if (u.isEmpty() || u.length() > 32) return null;
        if (!USERNAME_PATTERN.matcher(u).matches()) return null;
        return u;
    }

    private static String sanitizeMessage(String message) {
        if (message == null) return null;
        String m = message.replaceAll("\\p{Cntrl}", " ").trim();
        if (m.isEmpty()) return null;
        if (m.length() > MAX_MESSAGE_LEN) m = m.substring(0, MAX_MESSAGE_LEN);
        return m;
    }

    private static String htmlEscape(String s) {
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
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

    public static void main(String[] args) {
        // 5 Test cases
        // 1) Render main page initially
        System.out.println("=== Test 1: Initial main page ===");
        System.out.println(mainPage().substring(0, Math.min(200, mainPage().length())) + "...");

        // 2) Valid post
        System.out.println("=== Test 2: Valid post ===");
        System.out.println(post("Alice", "Hello World!"));

        // 3) XSS attempt post
        System.out.println("=== Test 3: XSS attempt ===");
        System.out.println(post("Bob", "<script>alert('x')</script>"));

        // 4) Invalid username
        System.out.println("=== Test 4: Invalid username ===");
        System.out.println(post("!!!", "Invalid user name should fail"));

        // 5) Render main page after posts
        System.out.println("=== Test 5: Main page after posts ===");
        String page = mainPage();
        System.out.println(page.substring(0, Math.min(400, page.length())) + "...");
    }
}