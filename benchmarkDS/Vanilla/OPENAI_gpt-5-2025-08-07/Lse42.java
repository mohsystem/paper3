import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Lse42 {
    private static final Map<String, Map<String, String>> SESSIONS = new ConcurrentHashMap<>();

    public static String info(Map<String, String> session) {
        String username = session.getOrDefault("username", "");
        String email = session.getOrDefault("email", "");
        String ssn = session.getOrDefault("ssn", "");
        return "username: " + username + "; email: " + email + "; ssn: " + ssn;
    }

    private static String getCookieSid(String cookieHeader) {
        if (cookieHeader == null) return null;
        String[] cookies = cookieHeader.split(";");
        for (String c : cookies) {
            String[] kv = c.trim().split("=", 2);
            if (kv.length == 2 && kv[0].trim().equals("SID")) {
                return kv[1].trim();
            }
        }
        return null;
    }

    private static String ensureSession(HttpExchange exchange) {
        Headers headers = exchange.getRequestHeaders();
        String cookie = headers.getFirst("Cookie");
        String sid = getCookieSid(cookie);
        boolean isNew = false;
        if (sid == null || !SESSIONS.containsKey(sid)) {
            sid = UUID.randomUUID().toString().replace("-", "");
            Map<String, String> sess = new HashMap<>();
            sess.put("username", "guest");
            sess.put("email", "guest@example.com");
            sess.put("ssn", "000-00-0000");
            SESSIONS.put(sid, sess);
            isNew = true;
        }
        if (isNew) {
            exchange.getResponseHeaders().add("Set-Cookie", "SID=" + sid + "; Path=/; HttpOnly");
        }
        return sid;
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            if (!"/info".equals(exchange.getRequestURI().getPath())) {
                exchange.sendResponseHeaders(404, -1);
                return;
            }
            String sid = ensureSession(exchange);
            Map<String, String> session = SESSIONS.get(sid);
            String body = info(session);
            byte[] bytes = body.getBytes("UTF-8");
            Headers resp = exchange.getResponseHeaders();
            resp.add("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for info()
        List<Map<String, String>> tests = new ArrayList<>();
        tests.add(new HashMap<>(Map.of("username", "alice", "email", "alice@example.com", "ssn", "111-22-3333")));
        tests.add(new HashMap<>(Map.of("username", "bob", "email", "bob@example.com", "ssn", "222-33-4444")));
        tests.add(new HashMap<>(Map.of("username", "carol", "email", "carol@example.com", "ssn", "333-44-5555")));
        tests.add(new HashMap<>(Map.of("username", "dave", "email", "dave@example.com", "ssn", "444-55-6666")));
        tests.add(new HashMap<>(Map.of("username", "eve", "email", "eve@example.com", "ssn", "555-66-7777")));

        for (int i = 0; i < tests.size(); i++) {
            String out = info(tests.get(i));
            System.out.println("Test " + (i + 1) + ": " + out);
        }

        // Start simple HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://localhost:8080/info");
    }
}