import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Lse40 {

    // info() called when the route is accessed; accepts session map and returns HTML page
    public static String info(Map<String, String> session) {
        String username = session.getOrDefault("username", "guest");
        String email = session.getOrDefault("email", "guest@example.com");
        String ssn = session.getOrDefault("ssn", "");
        String digits = ssn.replaceAll("\\D", "");
        String last4 = digits.length() == 0 ? "N/A" : digits.substring(Math.max(0, digits.length() - 4));
        StringBuilder sb = new StringBuilder();
        sb.append("<!DOCTYPE html><html><head><title>Info</title></head><body>");
        sb.append("<h1>User Info</h1>");
        sb.append("<p>Username: ").append(escapeHtml(username)).append("</p>");
        sb.append("<p>Email: ").append(escapeHtml(email)).append("</p>");
        sb.append("<p>SSN (last 4): ").append(escapeHtml(last4)).append("</p>");
        sb.append("</body></html>");
        return sb.toString();
    }

    private static String escapeHtml(String s) {
        return s == null ? "" : s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                .replace("\"", "&quot;").replace("'", "&#39;");
    }

    // Simple in-memory session manager using a SID cookie
    static class SessionManager {
        private final Map<String, Map<String, String>> store = new ConcurrentHashMap<>();
        private final String cookieName = "SID";

        public Map<String, String> getOrCreate(HttpExchange exchange) {
            String sid = getSidFromRequest(exchange.getRequestHeaders());
            boolean isNew = false;
            if (sid == null || !store.containsKey(sid)) {
                sid = UUID.randomUUID().toString();
                store.put(sid, new ConcurrentHashMap<>());
                isNew = true;
            }
            if (isNew) {
                Headers headers = exchange.getResponseHeaders();
                headers.add("Set-Cookie", cookieName + "=" + sid + "; Path=/; HttpOnly");
            }
            // Pre-seed default values if not present
            Map<String, String> sess = store.get(sid);
            sess.putIfAbsent("username", "guest");
            sess.putIfAbsent("email", "guest@example.com");
            sess.putIfAbsent("ssn", "0000");
            return sess;
        }

        private String getSidFromRequest(Headers headers) {
            List<String> cookies = headers.get("Cookie");
            if (cookies == null) return null;
            for (String c : cookies) {
                String[] parts = c.split(";");
                for (String part : parts) {
                    String[] kv = part.trim().split("=", 2);
                    if (kv.length == 2 && kv[0].equals(cookieName)) {
                        return kv[1];
                    }
                }
            }
            return null;
        }
    }

    static class InfoHandler implements HttpHandler {
        private final SessionManager sessionManager;

        InfoHandler(SessionManager sm) {
            this.sessionManager = sm;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                byte[] resp = "Method Not Allowed".getBytes();
                exchange.sendResponseHeaders(405, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }
            String path = exchange.getRequestURI().getPath();
            if (!"/info".equals(path)) {
                byte[] resp = "Not Found".getBytes();
                exchange.sendResponseHeaders(404, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }
            Map<String, String> session = sessionManager.getOrCreate(exchange);
            String body = info(session);
            byte[] bytes = body.getBytes();
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    // Starts a simple HTTP server with a single /info route
    public static HttpServer startServer(int port) throws IOException {
        SessionManager sm = new SessionManager();
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler(sm));
        server.setExecutor(null);
        server.start();
        return server;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases calling info() directly with session-like maps
        List<Map<String, String>> tests = new ArrayList<>();
        tests.add(new HashMap<String, String>() {{
            put("username", "alice");
            put("email", "alice@example.com");
            put("ssn", "123-45-6789");
        }});
        tests.add(new HashMap<String, String>() {{
            put("username", "bob");
            put("email", "bob@x.com");
            put("ssn", "0000");
        }});
        tests.add(new HashMap<String, String>() {{
            put("username", "carol");
            put("email", "carol@x.com");
            put("ssn", "987654321");
        }});
        tests.add(new HashMap<String, String>() {{
            put("username", "dave");
            put("email", "dave@x.com");
            put("ssn", "");
        }});
        tests.add(new HashMap<String, String>() {{
            put("username", "eve");
            put("email", "eve@x.com");
            put("ssn", "12-34");
        }});

        for (int i = 0; i < tests.size(); i++) {
            String out = info(tests.get(i));
            System.out.println("TEST " + (i + 1) + " OUTPUT:");
            System.out.println(out);
            System.out.println("-----");
        }

        // Start the web server (optional)
        // HttpServer server = startServer(8080);
        // System.out.println("Server started on http://localhost:8080/info");
        // Keep running:
        // Thread.currentThread().join();
    }
}