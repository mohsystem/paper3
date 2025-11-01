import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse138 {

    // Connect to the database (SQLite in-memory) and initialize schema/data
    public static Connection connectAndInitDb() throws Exception {
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException ignored) {
            // If driver is not on classpath, it will fail at runtime; for demonstration purposes only.
        }
        Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:");
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT, is_moderator INTEGER)");
            st.execute("INSERT INTO users (id, username, is_moderator) VALUES (1, 'alice', 1)");
            st.execute("INSERT INTO users (id, username, is_moderator) VALUES (2, 'bob', 0)");
            st.execute("INSERT INTO users (id, username, is_moderator) VALUES (3, 'carol', 1)");
            st.execute("INSERT INTO users (id, username, is_moderator) VALUES (4, 'dave', 0)");
            st.execute("INSERT INTO users (id, username, is_moderator) VALUES (5, 'eve', 0)");
        }
        return conn;
    }

    // Execute a query to check if the user is a moderator
    public static boolean isModerator(Connection conn, long userId) throws SQLException {
        String sql = "SELECT is_moderator FROM users WHERE id = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setLong(1, userId);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return rs.getInt(1) == 1;
                }
            }
        }
        return false;
    }

    // Define a route and handler for the web app
    public static HttpServer startServer(Connection conn, int port) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/isModerator", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        exchange.sendResponseHeaders(405, -1);
                        return;
                    }
                    Map<String, String> params = parseQueryParams(exchange.getRequestURI().getRawQuery());
                    String userIdStr = params.get("userId");
                    boolean result = false;
                    if (userIdStr != null) {
                        try {
                            long userId = Long.parseLong(userIdStr);
                            result = isModerator(conn, userId);
                        } catch (Exception ignored) {
                        }
                    }
                    byte[] resp = String.valueOf(result).getBytes(StandardCharsets.UTF_8);
                    Headers h = exchange.getResponseHeaders();
                    h.add("Content-Type", "text/plain; charset=utf-8");
                    exchange.sendResponseHeaders(200, resp.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(resp);
                    }
                } catch (Exception e) {
                    try {
                        exchange.sendResponseHeaders(500, -1);
                    } catch (Exception ignored) {
                    }
                } finally {
                    exchange.close();
                }
            }
        });
        server.start();
        return server;
    }

    private static Map<String, String> parseQueryParams(String raw) {
        Map<String, String> map = new HashMap<>();
        if (raw == null || raw.isEmpty()) return map;
        for (String pair : raw.split("&")) {
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                String k = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8);
                String v = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8);
                map.put(k, v);
            } else {
                map.put(URLDecoder.decode(pair, StandardCharsets.UTF_8), "");
            }
        }
        return map;
    }

    // Main method with 5 test cases
    public static void main(String[] args) throws Exception {
        Connection conn = connectAndInitDb();
        HttpServer server = startServer(conn, 8080);
        System.out.println("Java server started on http://localhost:8080/isModerator?userId={id}");

        long[] testIds = {1, 2, 3, 4, 999};
        for (long id : testIds) {
            boolean res = isModerator(conn, id);
            System.out.println("Test userId=" + id + " -> isModerator=" + res);
        }

        // Keep server running; press Ctrl+C to stop in real use
        // Here we just sleep a bit to allow manual testing if run.
        Thread.sleep(3000);
        server.stop(0);
        conn.close();
    }
}