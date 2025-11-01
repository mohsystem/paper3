import java.util.*;
import java.util.regex.Pattern;
import java.text.SimpleDateFormat;

/*
 Chain-of-Through in code (comments-only)
 1) Problem understanding: Implement post() to get username/message from request, insert via insert_user_message_in_db(), redirect, and display messages.
 2) Security requirements: Validate input, bound lengths, escape HTML for display, avoid control chars, avoid global mutable state races.
 3) Secure coding generation: Use synchronized list, immutable snapshots for rendering, regex validation, HTML escaping, length checks.
 4) Code review: Minimize exposure, no sensitive prints, consistent null-safety, handle failures with safe redirect.
 5) Secure code output: Final code reflects mitigations and safe behaviors.
*/

public class Lse148 {

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.\\- ]{1,32}$");
    private static final int MAX_MESSAGE_LEN = 1000;

    private static final List<Message> DB = Collections.synchronizedList(new ArrayList<>());

    private static class Message {
        final String usernameEsc;
        final String messageEsc;
        final long timestamp;
        Message(String u, String m, long ts) {
            this.usernameEsc = u;
            this.messageEsc = m;
            this.timestamp = ts;
        }
    }

    public static String post(Map<String, String> request) {
        // Get parameters safely
        String rawUser = safeTrim(request.getOrDefault("username", ""));
        String rawMsg = safeTrim(request.getOrDefault("message", ""));

        boolean ok = insert_user_message_in_db(rawUser, rawMsg);
        if (ok) {
            return "REDIRECT: /";
        } else {
            // Do not leak specifics; generic error redirect
            return "REDIRECT: /?error=invalid";
        }
    }

    public static boolean insert_user_message_in_db(String username, String message) {
        if (username == null || message == null) return false;
        username = safeTrim(username);
        message = safeTrim(message);

        if (!isValidUsername(username)) return false;
        if (!isValidMessage(message)) return false;

        // Escape for safe display
        String userEsc = escapeHtml(username);
        String msgEsc = escapeHtml(stripDangerousControls(message));

        Message m = new Message(userEsc, msgEsc, System.currentTimeMillis());
        synchronized (DB) {
            DB.add(m);
        }
        return true;
    }

    public static String display_messages() {
        List<Message> snapshot;
        synchronized (DB) {
            snapshot = new ArrayList<>(DB);
        }
        StringBuilder sb = new StringBuilder();
        sb.append("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>");
        sb.append("<h1>Messages</h1><ul>");
        SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        for (Message m : snapshot) {
            String ts = fmt.format(new Date(m.timestamp));
            sb.append("<li><strong>")
              .append(m.usernameEsc)
              .append("</strong> [")
              .append(ts)
              .append("]: ")
              .append(m.messageEsc)
              .append("</li>");
        }
        sb.append("</ul></body></html>");
        return sb.toString();
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        return USERNAME_PATTERN.matcher(u).matches();
    }

    private static boolean isValidMessage(String m) {
        if (m == null) return false;
        if (m.length() == 0 || m.length() > MAX_MESSAGE_LEN) return false;
        return true;
    }

    private static String safeTrim(String s) {
        return s == null ? "" : s.trim();
    }

    private static String stripDangerousControls(String s) {
        StringBuilder out = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if ((c == '\n') || (c == '\r') || (c == '\t') || !Character.isISOControl(c)) {
                out.append(c);
            }
        }
        return out.toString();
    }

    private static String escapeHtml(String s) {
        StringBuilder out = new StringBuilder((int)(s.length() * 1.2));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default:
                    if (Character.isISOControl(c) && c != '\n' && c != '\r' && c != '\t') {
                        // skip other control chars
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        Map<String, String> r1 = new HashMap<>();
        r1.put("username", "Alice");
        r1.put("message", "Hello world!");
        System.out.println(post(r1));

        Map<String, String> r2 = new HashMap<>();
        r2.put("username", "Bob_123");
        r2.put("message", "<script>alert('xss')</script> Hi!");
        System.out.println(post(r2));

        Map<String, String> r3 = new HashMap<>();
        r3.put("username", "Invalid#Name");
        r3.put("message", "Should be rejected");
        System.out.println(post(r3));

        Map<String, String> r4 = new HashMap<>();
        r4.put("username", "Carol");
        r4.put("message", "Line1\nLine2\tTabbed");
        System.out.println(post(r4));

        Map<String, String> r5 = new HashMap<>();
        r5.put("username", "Dave");
        char[] big = new char[1001];
        Arrays.fill(big, 'A');
        r5.put("message", new String(big)); // too long, reject
        System.out.println(post(r5));

        // Display messages
        System.out.println(display_messages());
    }
}