import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse138 {
    // Simple, safe "database" abstraction
    interface SafeDatabase {
        boolean connect(); // Simulate a connection step (e.g., read config, initialize)
        boolean executeIsModeratorQuery(String username); // "Query" the DB for moderator role
        void upsertUser(String username, String role);
    }

    static final class InMemoryDatabase implements SafeDatabase {
        private final Map<String, String> roles = new ConcurrentHashMap<>();
        private volatile boolean connected = false;

        @Override
        public boolean connect() {
            // In real life, would use environment variables for credentials/URL and use JDBC with prepared statements
            // Here we simulate a connection being established securely.
            connected = true;
            return true;
        }

        @Override
        public boolean executeIsModeratorQuery(String username) {
            if (!connected) return false;
            // Case-sensitive exact match; avoid any dynamic code execution
            String role = roles.get(username);
            return "moderator".equals(role);
        }

        @Override
        public void upsertUser(String username, String role) {
            if (username == null || role == null) return;
            roles.put(username, role);
        }
    }

    private static final InMemoryDatabase DB = new InMemoryDatabase();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,32}$");

    static class IsModeratorHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                byte[] resp = "{\"error\":\"method not allowed\"}".getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
                exchange.sendResponseHeaders(405, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }

            // Parse query
            URI requestURI = exchange.getRequestURI();
            String rawQuery = requestURI.getRawQuery(); // Do not decode before validation
            Map<String, String> q = parseQuery(rawQuery);

            String username = q.getOrDefault("username", "");
            if (!isValidUsername(username)) {
                byte[] resp = "{\"error\":\"invalid username\"}".getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
                exchange.sendResponseHeaders(400, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }

            // "Execute query"
            boolean isMod = DB.executeIsModeratorQuery(username);
            String json = "{\"username\":\"" + jsonEscape(username) + "\",\"is_moderator\":" + (isMod ? "true" : "false") + "}";
            byte[] resp = json.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
            exchange.sendResponseHeaders(200, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        // Decode percent-encoding safely after base checks
        try {
            String decoded = URLDecoder.decode(u, StandardCharsets.UTF_8.name());
            return USERNAME_PATTERN.matcher(decoded).matches();
        } catch (IllegalArgumentException | UnsupportedEncodingException e) {
            return false;
        }
    }

    private static Map<String, String> parseQuery(String rawQuery) {
        Map<String, String> map = new HashMap<>();
        if (rawQuery == null || rawQuery.length() > 2048) { // simple limit
            return map;
        }
        String[] pairs = rawQuery.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            if (idx <= 0) continue;
            String key = pair.substring(0, idx);
            String val = pair.substring(idx + 1);
            if ("username".equals(key)) {
                map.put("username", val);
            }
        }
        return map;
    }

    private static String jsonEscape(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (char c : s.toCharArray()) {
            switch (c) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    // Simple HTTP GET client for tests
    private static String httpGet(String url) throws IOException {
        HttpURLConnection conn = null;
        try {
            URL u = new URL(url);
            conn = (HttpURLConnection) u.openConnection();
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(3000);
            conn.setReadTimeout(3000);
            conn.setInstanceFollowRedirects(false);
            int code = conn.getResponseCode();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(
                (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream(),
                StandardCharsets.UTF_8))) {
                StringBuilder sb = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) sb.append(line);
                return sb.toString();
            }
        } finally {
            if (conn != null) conn.disconnect();
        }
    }

    public static void main(String[] args) throws Exception {
        // "Connect to database"
        if (!DB.connect()) {
            System.err.println("Database connection failed");
            return;
        }
        // Seed data (would be from a real DB in production)
        DB.upsertUser("alice", "moderator");
        DB.upsertUser("bob", "user");
        DB.upsertUser("carol", "moderator");
        DB.upsertUser("dave", "user");
        DB.upsertUser("eve", "moderator");

        // Start web app
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/is_moderator", new IsModeratorHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Server started at http://127.0.0.1:8080");

        // 5 test cases
        String[] tests = new String[]{"alice", "bob", "carol", "dave", "unknown"};
        for (String t : tests) {
            String url = "http://127.0.0.1:8080/is_moderator?username=" + URLEncoder.encode(t, StandardCharsets.UTF_8.name());
            String resp = httpGet(url);
            System.out.println(resp);
        }

        // Stop server
        server.stop(0);
    }
}