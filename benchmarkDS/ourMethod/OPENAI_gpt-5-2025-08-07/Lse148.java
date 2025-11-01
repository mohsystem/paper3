import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.regex.Pattern;

public class Lse148 {
    private static final int MAX_USERNAME_LEN = 32;
    private static final int MAX_MESSAGE_LEN = 280;
    private static final int MAX_MESSAGES = 1000;

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,32}$");
    private static final List<Message> DB = Collections.synchronizedList(new ArrayList<>());

    private static final class Message {
        final String username;
        final String message;
        final Instant createdAt;
        Message(String u, String m) {
            this.username = u;
            this.message = m;
            this.createdAt = Instant.now();
        }
    }

    public static String post(String username, String message) {
        String u = username == null ? "" : username.trim();
        String m = message == null ? "" : sanitizeMessage(message);

        if (!validateUsername(u)) {
            return "ERROR: invalid username";
        }
        if (!validateMessage(m)) {
            return "ERROR: invalid message";
        }
        boolean ok = insert_user_message_in_db(u, m);
        if (!ok) {
            return "ERROR: internal error";
        }
        return "REDIRECT: /";
    }

    public static boolean insert_user_message_in_db(String username, String message) {
        // Maintain a bounded in-memory log
        synchronized (DB) {
            if (DB.size() >= MAX_MESSAGES) {
                DB.remove(0);
            }
            DB.add(new Message(username, message));
        }
        return true;
    }

    public static String displayMessages() {
        StringBuilder sb = new StringBuilder();
        synchronized (DB) {
            for (Message msg : DB) {
                sb.append(safeDisplay(msg.username))
                  .append(": ")
                  .append(safeDisplay(msg.message))
                  .append(System.lineSeparator());
            }
        }
        return sb.toString();
    }

    private static boolean validateUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > MAX_USERNAME_LEN) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static String sanitizeMessage(String message) {
        // Remove control chars except 
 and \t, trim to MAX_MESSAGE_LEN
        if (message == null) return "";
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < message.length(); i++) {
            char c = message.charAt(i);
            if (c == '\n' || c == '\t' || (c >= 0x20 && c != 0x7F)) {
                sb.append(c);
                count++;
                if (count >= MAX_MESSAGE_LEN) break;
            }
        }
        return sb.toString().trim();
    }

    private static boolean validateMessage(String message) {
        if (message == null) return false;
        if (message.isEmpty()) return false;
        byte[] bytes = message.getBytes(StandardCharsets.UTF_8);
        return bytes.length <= MAX_MESSAGE_LEN;
    }

    private static String safeDisplay(String s) {
        // For console display we just strip CR; if used in HTML, apply escaping.
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c != '\r') sb.append(c);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test 1: valid
        System.out.println(post("alice", "Hello world!"));
        // Test 2: long message trimmed/sanitized
        StringBuilder longMsg = new StringBuilder();
        for (int i = 0; i < 400; i++) longMsg.append('x');
        System.out.println(post("bob_123", longMsg.toString()));
        // Test 3: invalid username
        System.out.println(post("bad user!", "This should fail"));
        // Test 4: empty message
        System.out.println(post("charlie", "   "));
        // Test 5: message with control and HTML
        System.out.println(post("dana", "<script>alert(1)</script>\u0001\u0002ok"));
        // Display
        System.out.println("=== Messages ===");
        System.out.print(displayMessages());
    }
}