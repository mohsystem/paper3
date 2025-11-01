import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Lse149 {

    // Message entity
    static final class Message {
        final String username;
        final String message;

        Message(String username, String message) {
            this.username = username;
            this.message = message;
        }
    }

    // Very simple in-memory "DB"
    static final class InMemoryDB {
        private final List<Message> messages = new ArrayList<>();
        private final int maxMessages;

        InMemoryDB(int maxMessages) {
            this.maxMessages = Math.max(1, maxMessages);
        }

        synchronized boolean insert_user_message_in_db(String username, String message) {
            // Input validation per security rules
            if (username == null || message == null) return false;
            if (!isValidUsername(username)) return false;
            if (!isValidMessage(message)) return false;

            if (messages.size() >= maxMessages) {
                // Evict oldest to bound memory
                messages.remove(0);
            }
            messages.add(new Message(username, message));
            return true;
        }

        synchronized List<Message> getAll() {
            return new ArrayList<>(messages);
        }

        private static boolean isValidUsername(String s) {
            if (s.length() < 1 || s.length() > 20) return false;
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                if (!(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                    return false;
            }
            return true;
        }

        private static boolean isValidMessage(String s) {
            int len = s.codePointCount(0, s.length());
            return len >= 1 && len <= 280;
        }
    }

    // Simple Request/Response model to simulate routing
    static final class Request {
        final String method;
        final String path;
        final Map<String, String> form;

        Request(String method, String path, Map<String, String> form) {
            this.method = method;
            this.path = path;
            this.form = form == null ? Collections.emptyMap() : new HashMap<>(form);
        }
    }

    static final class Response {
        final int statusCode;
        final Map<String, String> headers;
        final byte[] body;

        Response(int statusCode, Map<String, String> headers, byte[] body) {
            this.statusCode = statusCode;
            this.headers = headers == null ? new HashMap<>() : new HashMap<>(headers);
            this.body = body == null ? new byte[0] : body;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Status: ").append(statusCode).append("\n");
            for (Map.Entry<String, String> e : headers.entrySet()) {
                sb.append(e.getKey()).append(": ").append(e.getValue()).append("\n");
            }
            sb.append("\n");
            sb.append(new String(body, StandardCharsets.UTF_8));
            return sb.toString();
        }
    }

    // Router and handlers
    static final class App {
        private final InMemoryDB db;

        App(InMemoryDB db) {
            this.db = db;
        }

        public Response handle(Request req) {
            if (req == null || req.method == null || req.path == null) {
                return text(400, "Bad Request");
            }
            if ("GET".equals(req.method) && "/".equals(req.path)) {
                return handleRoot();
            } else if ("POST".equals(req.method) && "/post".equals(req.path)) {
                return handlePost(req);
            }
            return text(404, "Not Found");
        }

        private Response handleRoot() {
            StringBuilder html = new StringBuilder();
            html.append("<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>");
            html.append("<h1>Message Board</h1>");
            html.append("<form method=\"POST\" action=\"/post\" onsubmit=\"return validate();\">");
            html.append("Username: <input name=\"username\" maxlength=\"20\" required pattern=\"[A-Za-z0-9_]+\"> ");
            html.append("Message: <input name=\"message\" maxlength=\"280\" required> ");
            html.append("<button type=\"submit\">Post</button></form>");
            html.append("<script>function validate(){var u=document.forms[0].username.value;if(!/^[A-Za-z0-9_]{1,20}$/.test(u))return false;return true;}</script>");
            html.append("<h2>Messages</h2><ul>");
            for (Message m : db.getAll()) {
                html.append("<li><strong>")
                    .append(escapeHtml(m.username))
                    .append(":</strong> ")
                    .append(escapeHtml(m.message))
                    .append("</li>");
            }
            html.append("</ul></body></html>");
            return html(200, html.toString());
        }

        private Response handlePost(Request req) {
            String username = safeTrim(req.form.get("username"));
            String message = safeTrim(req.form.get("message"));

            boolean ok = db.insert_user_message_in_db(username, message);
            if (!ok) {
                return text(400, "Invalid input");
            }
            Map<String, String> headers = new HashMap<>();
            headers.put("Location", "/");
            return new Response(303, headers, new byte[0]);
        }

        private static String safeTrim(String s) {
            return s == null ? null : s.trim();
        }

        private static String escapeHtml(String s) {
            if (s == null) return "";
            StringBuilder out = new StringBuilder(Math.min(4096, s.length() * 2));
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                switch (c) {
                    case '&': out.append("&amp;"); break;
                    case '<': out.append("&lt;"); break;
                    case '>': out.append("&gt;"); break;
                    case '"': out.append("&quot;"); break;
                    case '\'': out.append("&#39;"); break;
                    default:
                        if (c >= 0x20 || c == '\n' || c == '\t') out.append(c);
                        // else skip control chars
                }
            }
            return out.toString();
        }

        private static Response text(int code, String body) {
            Map<String, String> h = new HashMap<>();
            h.put("Content-Type", "text/plain; charset=utf-8");
            return new Response(code, h, body.getBytes(StandardCharsets.UTF_8));
        }

        private static Response html(int code, String body) {
            Map<String, String> h = new HashMap<>();
            h.put("Content-Type", "text/html; charset=utf-8");
            return new Response(code, h, body.getBytes(StandardCharsets.UTF_8));
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        InMemoryDB db = new InMemoryDB(1000);
        App app = new App(db);

        List<Runnable> tests = new ArrayList<>();
        AtomicInteger testNo = new AtomicInteger(1);

        // Test 1: Valid post then GET
        tests.add(() -> {
            Map<String, String> form = new HashMap<>();
            form.put("username", "alice");
            form.put("message", "Hello, world!");
            System.out.println("Test " + testNo.getAndIncrement() + " POST /post valid:");
            System.out.println(app.handle(new Request("POST", "/post", form)).toString());
            System.out.println("GET / after Test 1:");
            System.out.println(app.handle(new Request("GET", "/", null)).toString());
        });

        // Test 2: Invalid username
        tests.add(() -> {
            Map<String, String> form = new HashMap<>();
            form.put("username", "bob!"); // invalid char
            form.put("message", "This should fail.");
            System.out.println("Test " + testNo.getAndIncrement() + " POST /post invalid username:");
            System.out.println(app.handle(new Request("POST", "/post", form)).toString());
        });

        // Test 3: Message too long (>280)
        tests.add(() -> {
            char[] arr = new char[281];
            Arrays.fill(arr, 'x');
            Map<String, String> form = new HashMap<>();
            form.put("username", "charlie");
            form.put("message", new String(arr));
            System.out.println("Test " + testNo.getAndIncrement() + " POST /post too long message:");
            System.out.println(app.handle(new Request("POST", "/post", form)).toString());
        });

        // Test 4: XSS attempt; should be escaped on display
        tests.add(() -> {
            Map<String, String> form = new HashMap<>();
            form.put("username", "dana");
            form.put("message", "<script>alert('xss')</script> safe");
            System.out.println("Test " + testNo.getAndIncrement() + " POST /post XSS attempt:");
            System.out.println(app.handle(new Request("POST", "/post", form)).toString());
            System.out.println("GET / after Test 4:");
            System.out.println(app.handle(new Request("GET", "/", null)).toString());
        });

        // Test 5: Another valid post and list
        tests.add(() -> {
            Map<String, String> form = new HashMap<>();
            form.put("username", "eve_123");
            form.put("message", "Good day!");
            System.out.println("Test " + testNo.getAndIncrement() + " POST /post valid again:");
            System.out.println(app.handle(new Request("POST", "/post", form)).toString());
            System.out.println("GET / after Test 5:");
            System.out.println(app.handle(new Request("GET", "/", null)).toString());
        });

        for (Runnable t : tests) {
            t.run();
            System.out.println("------------------------------------------------------------");
        }
    }
}