import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse138 {

    // Simple in-memory "database" connection
    static class DBConnection {
        private final Map<String, Boolean> users = new HashMap<>();
        private boolean connected = false;

        public boolean connect() {
            // Simulate connecting to a DB
            connected = true;
            return connected;
        }

        public void seed(Map<String, Boolean> seedData) {
            users.clear();
            users.putAll(seedData);
        }

        // Simulate executing a query: SELECT is_moderator FROM users WHERE user_id = ?
        public Optional<Boolean> executeQueryIsModerator(String userId) {
            if (!connected) throw new IllegalStateException("DB not connected");
            if (userId == null) return Optional.empty();
            return Optional.ofNullable(users.get(userId));
        }
    }

    // Core query function
    public static boolean isModerator(DBConnection db, String userId) {
        return db.executeQueryIsModerator(userId).orElse(false);
    }

    // Route handler function (called when route is invoked)
    public static String handleRoute(DBConnection db, String userId) {
        boolean result = isModerator(db, userId);
        return Boolean.toString(result);
    }

    // Start a minimal HTTP server with a route: /isModerator?userId=123
    public static HttpServer startServer(DBConnection db, int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/isModerator", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    return;
                }
                URI uri = exchange.getRequestURI();
                String query = uri.getQuery();
                Map<String, String> params = parseQuery(query);
                String userId = params.getOrDefault("userId", null);
                String response = handleRoute(db, userId);
                byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
            }
        });
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        return server;
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        for (String pair : query.split("&")) {
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                String key = urlDecode(pair.substring(0, idx));
                String val = urlDecode(pair.substring(idx + 1));
                map.put(key, val);
            } else {
                map.put(urlDecode(pair), "");
            }
        }
        return map;
    }

    private static String urlDecode(String s) {
        return URLDecoder.decode(s, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) throws Exception {
        DBConnection db = new DBConnection();
        db.connect();
        Map<String, Boolean> seed = new HashMap<>();
        seed.put("1", true);
        seed.put("2", false);
        seed.put("3", true);
        seed.put("4", false);
        seed.put("5", true);
        db.seed(seed);

        // Start web app
        HttpServer server = startServer(db, 8081);

        // 5 test cases (directly invoking the route handler function)
        System.out.println(handleRoute(db, "1")); // expected: true
        System.out.println(handleRoute(db, "2")); // expected: false
        System.out.println(handleRoute(db, "3")); // expected: true
        System.out.println(handleRoute(db, "4")); // expected: false
        System.out.println(handleRoute(db, "999")); // expected: false

        // Optional: keep server running for demonstration. Comment next line to keep running.
        server.stop(0);
    }
}