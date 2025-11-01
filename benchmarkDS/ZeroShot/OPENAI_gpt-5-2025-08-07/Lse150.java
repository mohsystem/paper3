import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;

public class Lse150 {
    // In-memory "DB" placeholder for messages (username: message)
    private static final List<String> MESSAGES_DB = new ArrayList<>();

    // HTML escape to prevent XSS when rendering user-provided content
    public static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length());
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                case '/': sb.append("&#x2F;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Validate username: 3-32 chars, alphanum, underscore, hyphen
    public static boolean validateUsername(String username) {
        if (username == null) return false;
        String u = username.trim();
        if (u.length() < 3 || u.length() > 32) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isAlphabetic(c) || Character.isDigit(c) || c == '_' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    // Sanitize message: normalize, trim, remove control chars except newline, enforce length <= 500
    public static String sanitizeMessage(String message) {
        if (message == null) return null;
        String m = Normalizer.normalize(message, Normalizer.Form.NFKC).trim();
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < m.length(); i++) {
            char c = m.charAt(i);
            if (c == '\n' || (c >= 0x20 && c != 0x7F)) {
                sb.append(c);
                count++;
                if (count > 500) {
                    return null; // too long
                }
            }
        }
        if (sb.length() == 0) {
            return null; // empty after sanitization
        }
        return sb.toString();
    }

    // Constant-time comparison for CSRF tokens
    public static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        return MessageDigest.isEqual(x, y);
    }

    // Secure CSRF token generator
    public static String generateCsrfToken() {
        byte[] bytes = new byte[32];
        new SecureRandom().nextBytes(bytes);
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte byt : bytes) {
            sb.append(String.format("%02x", byt));
        }
        return sb.toString();
    }

    // Simulates inserting into DB using safe parameters
    public static boolean insert_user_message_in_db(String username, String message) {
        // In a real application, use a prepared statement to prevent SQL injection:
        // String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        // PreparedStatement ps = conn.prepareStatement(sql);
        // ps.setString(1, username);
        // ps.setString(2, message);
        // ps.executeUpdate();
        MESSAGES_DB.add(username + ": " + message);
        return true;
    }

    // Renders main page HTML with a secure form
    public static String render_main_page(String csrfToken) {
        String safeToken = htmlEscape(csrfToken);
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Main</title></head><body>");
        html.append("<h1>Submit a message</h1>");
        html.append("<form method=\"post\" action=\"/post\" autocomplete=\"off\">");
        html.append("<label for=\"username\">Username</label>");
        html.append("<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"3\" maxlength=\"32\" pattern=\"[A-Za-z0-9_\\-]+\">");
        html.append("<br>");
        html.append("<label for=\"message\">Message</label>");
        html.append("<textarea id=\"message\" name=\"message\" required maxlength=\"500\"></textarea>");
        html.append("<input type=\"hidden\" name=\"csrf\" value=\"").append(safeToken).append("\">");
        html.append("<br><button type=\"submit\">Send</button>");
        html.append("</form>");
        html.append("</body></html>");
        return html.toString();
    }

    // Handles POST submission: validates, inserts, and redirects
    public static String post(String username, String message, String csrfTokenFromRequest, String sessionCsrfToken) {
        if (!constantTimeEquals(csrfTokenFromRequest, sessionCsrfToken)) {
            return "REDIRECT:/?error=csrf";
        }
        if (!validateUsername(username)) {
            return "REDIRECT:/?error=bad_username";
        }
        String sanitized = sanitizeMessage(message);
        if (sanitized == null) {
            return "REDIRECT:/?error=bad_message";
        }
        boolean ok = insert_user_message_in_db(username.trim(), sanitized);
        if (!ok) {
            return "REDIRECT:/?error=server";
        }
        return "REDIRECT:/";
    }

    public static void main(String[] args) {
        String sessionToken = generateCsrfToken();
        System.out.println("Rendered Main Page HTML:");
        System.out.println(render_main_page(sessionToken));

        // Test 1: Valid submission
        System.out.println("Test1: " + post("alice_01", "Hello world!", sessionToken, sessionToken));

        // Test 2: Invalid username (too short)
        System.out.println("Test2: " + post("a", "short name", sessionToken, sessionToken));

        // Test 3: Message with HTML/script content
        System.out.println("Test3: " + post("bob-2", "<script>alert(1)</script>", sessionToken, sessionToken));

        // Test 4: Too long message
        StringBuilder longMsg = new StringBuilder();
        for (int i = 0; i < 600; i++) longMsg.append('x');
        System.out.println("Test4: " + post("charlie_3", longMsg.toString(), sessionToken, sessionToken));

        // Test 5: CSRF mismatch
        System.out.println("Test5: " + post("dave_4", "Valid message", "badtoken", sessionToken));

        // Show DB contents
        System.out.println("DB size: " + MESSAGES_DB.size());
        for (String rec : MESSAGES_DB) {
            System.out.println(rec);
        }
    }
}