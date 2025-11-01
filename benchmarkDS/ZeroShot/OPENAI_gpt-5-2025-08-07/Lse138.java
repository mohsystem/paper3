import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.Headers;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;

public class Lse138 {

    // Shared in-memory SQLite connection (shared cache)
    private static Connection sharedConn;
    private static final Pattern USER_PATTERN = Pattern.compile("^[A-Za-z0-9_]{1,32}$");

    public static void main(String[] args) throws Exception {
        // Initialize DB
        sharedConn = connect("jdbc:sqlite:file:memdb_java?mode=memory&cache=shared");
        initDb(sharedConn);

        // Start server
        HttpServer server = startServer(8080, sharedConn);

        // 5 test cases using HTTP client
        List<String> testUsers = Arrays.asList("alice", "bob", "charlie", "admin_1", "unknown_user");
        for (String u : testUsers) {
            String res = httpGet("http://127.0.0.1:8080/is_moderator?user=" + urlEncode(u));
            System.out.println(u + " -> " + res);
        }

        // Keep server alive briefly for demonstration then stop
        Thread.sleep(500);
        server.stop(0);
        if (sharedConn != null) sharedConn.close();
    }

    public static Connection connect(String jdbcUrl) throws SQLException {
        // Load SQLite JDBC driver if available
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException ignored) {}
        Connection conn = DriverManager.getConnection(jdbcUrl);
        conn.setAutoCommit(true);
        return conn;
    }

    public static void initDb(Connection conn) throws SQLException {
        String create = "CREATE TABLE IF NOT EXISTS users (" +
                "username TEXT PRIMARY KEY," +
                "is_moderator INTEGER NOT NULL CHECK(is_moderator IN (0,1))" +
                ")";
        try (Statement st = conn.createStatement()) {
            st.execute(create);
        }
        // Upsert test data securely
        upsertUser(conn, "alice", true);
        upsertUser(conn, "bob", false);
        upsertUser(conn, "charlie", false);
        upsertUser(conn, "admin_1", true);
    }

    public static void upsertUser(Connection conn, String username, boolean isModerator) throws SQLException {
        try (PreparedStatement ps = conn.prepareStatement(
                "INSERT INTO users(username, is_moderator) VALUES(?, ?) " +
                        "ON CONFLICT(username) DO UPDATE SET is_moderator=excluded.is_moderator")) {
            ps.setString(1, username);
            ps.setInt(2, isModerator ? 1 : 0);
            ps.executeUpdate();
        }
    }

    // Core function: accepts input as parameters and returns output
    public static boolean isModerator(Connection conn, String username) throws SQLException {
        if (username == null || !USER_PATTERN.matcher(username).matches()) {
            return false;
        }
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT is_moderator FROM users WHERE username = ?")) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return rs.getInt(1) == 1;
                }
            }
        }
        return false;
    }

    public static HttpServer startServer(int port, Connection conn) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/is_moderator", exchange -> handleIsModerator(exchange, conn));
        server.setExecutor(null);
        server.start();
        return server;
    }

    private static void handleIsModerator(HttpExchange exchange, Connection conn) throws IOException {
        if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
            sendJson(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
            return;
        }
        URI uri = exchange.getRequestURI();
        Map<String, String> params = parseQuery(uri.getRawQuery());
        String user = params.getOrDefault("user", "");
        boolean result = false;
        int status = 200;
        try {
            result = isModerator(conn, user);
        } catch (SQLException e) {
            status = 500;
        }
        String body = "{\"isModerator\":" + (result ? "true" : "false") + "}";
        sendJson(exchange, status, body);
    }

    private static void sendJson(HttpExchange exchange, int status, String body) throws IOException {
        Headers headers = exchange.getResponseHeaders();
        headers.set("Content-Type", "application/json; charset=utf-8");
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static Map<String, String> parseQuery(String rawQuery) {
        Map<String, String> map = new HashMap<>();
        if (rawQuery == null || rawQuery.isEmpty()) return map;
        String[] pairs = rawQuery.split("&");
        for (String p : pairs) {
            int i = p.indexOf('=');
            if (i > 0) {
                String k = urlDecode(p.substring(0, i));
                String v = urlDecode(p.substring(i + 1));
                map.put(k, v);
            }
        }
        return map;
    }

    private static String urlDecode(String s) {
        try {
            return java.net.URLDecoder.decode(s, "UTF-8");
        } catch (Exception e) {
            return "";
        }
    }

    private static String urlEncode(String s) {
        try {
            return java.net.URLEncoder.encode(s, "UTF-8");
        } catch (Exception e) {
            return s;
        }
    }

    private static String httpGet(String urlStr) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection c = (HttpURLConnection) url.openConnection();
        c.setRequestMethod("GET");
        c.setConnectTimeout(2000);
        c.setReadTimeout(2000);
        int code = c.getResponseCode();
        try (java.io.InputStream is = (code >= 200 && code < 300) ? c.getInputStream() : c.getErrorStream();
             java.util.Scanner sc = new java.util.Scanner(is, StandardCharsets.UTF_8.name())) {
            sc.useDelimiter("\\A");
            return sc.hasNext() ? sc.next() : "";
        }
    }
}