import java.util.ArrayList;
import java.util.List;

public class Lse149 {
    private static final int MAX_USER_LEN = 50;
    private static final int MAX_MSG_LEN = 500;

    private static final List<Message> DB = new ArrayList<>();

    private static class Message {
        final String username;
        final String message;
        Message(String u, String m) {
            this.username = u;
            this.message = m;
        }
    }

    private static String sanitize(String s, int maxLen) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < s.length() && count < maxLen; i++) {
            char c = s.charAt(i);
            if (c >= 32 && c != 127) {
                sb.append(c);
                count++;
            }
        }
        // trim leading/trailing whitespace
        return sb.toString().trim();
    }

    private static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(s.length());
        for (char c : s.toCharArray()) {
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

    // Simulated DB insert
    public static boolean insert_user_message_in_db(String username, String message) {
        String user = sanitize(username, MAX_USER_LEN);
        String msg = sanitize(message, MAX_MSG_LEN);
        if (user.isEmpty() || msg.isEmpty()) {
            return false;
        }
        synchronized (DB) {
            DB.add(new Message(user, msg));
        }
        return true;
    }

    // Simulated /post route handler
    public static String routePost(String username, String message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT: /";
    }

    // Simulated / route handler - display messages
    public static String routeRoot() {
        StringBuilder html = new StringBuilder();
        html.append("<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>");
        html.append("<h1>Messages</h1><ul>");
        synchronized (DB) {
            for (Message m : DB) {
                html.append("<li><strong>")
                    .append(htmlEscape(m.username))
                    .append("</strong>: ")
                    .append(htmlEscape(m.message))
                    .append("</li>");
            }
        }
        html.append("</ul></body></html>");
        return html.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(routePost("alice", "Hello, world!"));
        System.out.println(routePost("bob", "Testing <b>bold</b> & \"quotes\""));
        System.out.println(routePost("charlie", "Line1\nLine2 should be sanitized for control chars."));
        System.out.println(routePost("dave", "This is a very long message that will be trimmed if too long.".repeat(20)));
        System.out.println(routePost(" eve ", "   spaced input   "));

        System.out.println(routeRoot());
    }
}