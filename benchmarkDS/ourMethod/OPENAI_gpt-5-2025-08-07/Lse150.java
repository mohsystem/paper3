import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;

public class Lse150 {
    // In-memory "database"
    private static final List<Message> DB = new CopyOnWriteArrayList<>();
    private static final int MAX_USERNAME_LEN = 32;
    private static final int MAX_MESSAGE_LEN = 500;

    public static final class Message {
        public final String username;
        public final String message;
        public Message(String u, String m) { this.username = u; this.message = m; }
    }

    public static final class Request {
        private final Map<String, String> form;
        public Request(Map<String, String> form) {
            // defensively copy and clamp sizes
            this.form = new HashMap<>();
            for (Map.Entry<String, String> e : form.entrySet()) {
                String k = e.getKey() == null ? "" : e.getKey();
                String v = e.getValue() == null ? "" : e.getValue();
                // Clamp extremely large inputs to a maximum size to avoid memory abuse
                if (v.length() > 8192) v = v.substring(0, 8192);
                this.form.put(k, v);
            }
        }
        public String getParam(String key) {
            return form.getOrDefault(key, "");
        }
    }

    public static final class Response {
        public final int statusCode;
        public final Map<String, String> headers;
        public final String body;
        public Response(int code, Map<String, String> headers, String body) {
            this.statusCode = code;
            this.headers = headers;
            this.body = body;
        }
        @Override public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("HTTP/1.1 ").append(statusCode).append("\n");
            for (Map.Entry<String,String> e : headers.entrySet()) {
                sb.append(e.getKey()).append(": ").append(e.getValue()).append("\n");
            }
            sb.append("\n");
            if (body != null) sb.append(body);
            return sb.toString();
        }
    }

    // Secure input validation
    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() == 0 || u.length() > MAX_USERNAME_LEN) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') &&
                c != '-' && c != '_') {
                return false;
            }
        }
        return true;
    }

    private static String sanitizeMessage(String m) {
        if (m == null) return null;
        // normalize newlines and strip control chars except 
 and \t
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < m.length(); i++) {
            char c = m.charAt(i);
            // Allow printable ASCII and common unicode. Drop ISO control except 
 and \t.
            if ((c == '\n') || (c == '\t')) {
                sb.append(c);
                count++;
            } else if (!Character.isISOControl(c)) {
                sb.append(c);
                count++;
            }
            if (count > MAX_MESSAGE_LEN) {
                return null; // too long after sanitization
            }
        }
        String out = sb.toString().trim();
        if (out.isEmpty()) return null;
        return out;
    }

    private static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(Math.min(8192, s.length() * 2));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Simulated DB insert
    public static boolean insert_user_message_in_db(String username, String message) {
        try {
            DB.add(new Message(username, message));
            return true;
        } catch (RuntimeException ex) {
            return false;
        }
    }

    // The post() handler: validates, inserts, redirects to main page.
    public static Response post(Request req) {
        String rawUser = req.getParam("username");
        String rawMsg = req.getParam("message");

        // Trim leading/trailing whitespace early
        String username = rawUser == null ? "" : rawUser.trim();
        String sanitizedMessage = sanitizeMessage(rawMsg);

        if (!isValidUsername(username) || sanitizedMessage == null) {
            Map<String,String> headers = new HashMap<>();
            headers.put("Content-Type", "text/plain; charset=UTF-8");
            return new Response(400, headers, "Invalid input");
        }

        if (!insert_user_message_in_db(username, sanitizedMessage)) {
            Map<String,String> headers = new HashMap<>();
            headers.put("Content-Type", "text/plain; charset=UTF-8");
            return new Response(500, headers, "Internal error");
        }

        Map<String,String> headers = new HashMap<>();
        headers.put("Location", "/");
        return new Response(303, headers, ""); // redirect to main page
    }

    // Main page with form and message list
    public static String getMainPage() {
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"UTF-8\">")
            .append("<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">")
            .append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
            .append("<title>Secure Message Board</title></head><body>")
            .append("<h1>Message Board</h1>")
            .append("<form method=\"POST\" action=\"/post\" autocomplete=\"off\">")
            .append("<label>Username: <input type=\"text\" name=\"username\" required maxlength=\"")
            .append(MAX_USERNAME_LEN)
            .append("\" pattern=\"[A-Za-z0-9_-]+\"></label><br>")
            .append("<label>Message:<br><textarea name=\"message\" required maxlength=\"")
            .append(MAX_MESSAGE_LEN)
            .append("\" rows=\"4\" cols=\"40\"></textarea></label><br>")
            .append("<button type=\"submit\">Send</button>")
            .append("</form><hr><h2>Recent Messages</h2><ul>");
        for (Message m : DB) {
            html.append("<li><strong>")
                .append(htmlEscape(m.username))
                .append(":</strong> ")
                .append(htmlEscape(m.message))
                .append("</li>");
        }
        html.append("</ul></body></html>");
        return html.toString();
    }

    // Test cases
    public static void main(String[] args) {
        // 1) Valid post
        Map<String,String> f1 = new HashMap<>();
        f1.put("username", "alice");
        f1.put("message", "Hello, world!");
        System.out.println(post(new Request(f1)));

        // 2) Valid post with HTML needing escape
        Map<String,String> f2 = new HashMap<>();
        f2.put("username", "Bob_123");
        f2.put("message", "Hi <script>alert('x')</script> & welcome!");
        System.out.println(post(new Request(f2)));

        // 3) Invalid username
        Map<String,String> f3 = new HashMap<>();
        f3.put("username", "bad!user");
        f3.put("message", "This should fail.");
        System.out.println(post(new Request(f3)));

        // 4) Too long message
        StringBuilder longMsg = new StringBuilder();
        for (int i = 0; i < 600; i++) longMsg.append('x');
        Map<String,String> f4 = new HashMap<>();
        f4.put("username", "charlie");
        f4.put("message", longMsg.toString());
        System.out.println(post(new Request(f4)));

        // 5) Empty after trim
        Map<String,String> f5 = new HashMap<>();
        f5.put("username", "david");
        f5.put("message", "   \n\t   ");
        System.out.println(post(new Request(f5)));

        // Render main page HTML after successful posts
        String page = getMainPage();
        System.out.println("---- MAIN PAGE HTML ----");
        System.out.println(page);
        System.out.println("---- END PAGE ----");
    }
}