import java.util.*;
import java.time.Instant;
import java.util.regex.Pattern;

public class Lse148 {

    private static final int MAX_MESSAGE_LEN = 500;
    private static final int MAX_USERNAME_LEN = 32;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,32}$");

    private static final List<Message> DB = Collections.synchronizedList(new ArrayList<>());

    private static class Message {
        final String username;
        final String message;
        final long timestamp;

        Message(String username, String message, long timestamp) {
            this.username = username;
            this.message = message;
            this.timestamp = timestamp;
        }
    }

    public static String post(String username, String message) {
        String safeUsername = sanitizeUsername(username);
        String safeMessage = sanitizeMessage(message);

        if (safeUsername == null || safeMessage == null) {
            return "redirect:/";
        }

        insert_user_message_in_db(safeUsername, safeMessage);
        return "redirect:/";
    }

    private static void insert_user_message_in_db(String username, String message) {
        Message m = new Message(username, message, Instant.now().getEpochSecond());
        DB.add(m);
    }

    public static String displayMessages() {
        StringBuilder sb = new StringBuilder();
        sb.append("<html><body>\n");
        sb.append("<h2>Messages</h2>\n<ul>\n");
        synchronized (DB) {
            for (Message m : DB) {
                sb.append("<li><strong>")
                  .append(htmlEscape(m.username))
                  .append(":</strong> ")
                  .append(htmlEscape(m.message))
                  .append("</li>\n");
            }
        }
        sb.append("</ul>\n</body></html>");
        return sb.toString();
    }

    public static String renderMainPage() {
        return displayMessages();
    }

    private static String sanitizeUsername(String username) {
        if (username == null) return null;
        String u = username.trim();
        if (u.length() == 0 || u.length() > MAX_USERNAME_LEN) return null;
        if (!USERNAME_PATTERN.matcher(u).matches()) return null;
        return u;
    }

    private static String sanitizeMessage(String message) {
        if (message == null) return null;
        String m = message.replace("\r\n", "\n").replace("\r", "\n").trim();
        if (m.length() == 0) return null;
        if (m.length() > MAX_MESSAGE_LEN) {
            m = m.substring(0, MAX_MESSAGE_LEN);
        }
        // Remove non-printable controls except tab and newline
        StringBuilder sb = new StringBuilder(m.length());
        for (int i = 0; i < m.length(); i++) {
            char c = m.charAt(i);
            if (c == '\n' || c == '\t' || (c >= 0x20 && c != 0x7F)) {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    private static String htmlEscape(String s) {
        StringBuilder out = new StringBuilder(s.length() + 16);
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
        // 5 test cases
        System.out.println(post("alice", "Hello, world!"));
        System.out.println(post("bob_42", "This is a test message."));
        System.out.println(post("charlie", "<script>alert('xss');</script> Stay safe!"));
        System.out.println(post("invalid name", "Should be rejected due to invalid username."));
        System.out.println(post("david", "")); // empty message rejected

        String page = renderMainPage();
        System.out.println(page);
    }
}