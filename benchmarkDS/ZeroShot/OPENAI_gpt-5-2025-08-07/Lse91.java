import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.sql.*;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse91 {
    // Configuration
    private static final int PORT = 8080;
    private static final int MAX_BODY_SIZE = 2048;
    private static final String DB_PATH = "users_java.db";
    private static final int ITERATIONS = 100_000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32;

    public static void main(String[] args) throws Exception {
        initDatabase(DB_PATH);
        // Start HTTP Server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 0);
        server.createContext("/register", new RegisterHandler(DB_PATH));
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Java server started on http://127.0.0.1:" + PORT);

        // 5 Test cases (direct function calls)
        System.out.println("[Test1] " + registerUser(DB_PATH, "alice_01", "StrongPass123!"));
        System.out.println("[Test2] " + registerUser(DB_PATH, "alice_01", "AnotherStrong123!")); // duplicate
        System.out.println("[Test3] " + registerUser(DB_PATH, "bad user!", "StrongPass123!")); // invalid username
        System.out.println("[Test4] " + registerUser(DB_PATH, "bob", "123")); // weak password
        System.out.println("[Test5] " + registerUser(DB_PATH, "charlie_02", "Y3tAn0ther$trong"));
    }

    static class RegisterHandler implements HttpHandler {
        private final String dbPath;

        RegisterHandler(String dbPath) {
            this.dbPath = dbPath;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                String contentType = Optional.ofNullable(exchange.getRequestHeaders().getFirst("Content-Type")).orElse("");
                if (!contentType.toLowerCase(Locale.ROOT).contains("application/x-www-form-urlencoded")) {
                    send(exchange, 415, "Unsupported Media Type");
                    return;
                }
                String cl = exchange.getRequestHeaders().getFirst("Content-Length");
                if (cl == null) {
                    send(exchange, 411, "Length Required");
                    return;
                }
                int length;
                try {
                    length = Integer.parseInt(cl);
                } catch (NumberFormatException e) {
                    send(exchange, 400, "Bad Request");
                    return;
                }
                if (length < 0 || length > MAX_BODY_SIZE) {
                    send(exchange, 413, "Payload Too Large");
                    return;
                }
                byte[] body = readN(exchange.getRequestBody(), length);
                String form = new String(body, StandardCharsets.UTF_8);
                Map<String, String> params = parseForm(form);
                String username = params.getOrDefault("username", "");
                String password = params.getOrDefault("password", "");
                String result = registerUser(dbPath, username, password);
                if (result.startsWith("OK")) {
                    send(exchange, 201, "Registered");
                } else if (result.contains("exists")) {
                    send(exchange, 409, "Conflict");
                } else {
                    send(exchange, 400, "Bad Request");
                }
            } catch (Exception e) {
                send(exchange, 500, "Internal Server Error");
            }
        }
    }

    private static void send(HttpExchange ex, int status, String msg) throws IOException {
        byte[] b = msg.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        ex.sendResponseHeaders(status, b.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(b);
        }
    }

    private static byte[] readN(InputStream is, int n) throws IOException {
        byte[] buf = new byte[n];
        int off = 0;
        while (off < n) {
            int r = is.read(buf, off, n - off);
            if (r == -1) break;
            off += r;
        }
        return Arrays.copyOf(buf, off);
    }

    private static Map<String, String> parseForm(String form) throws UnsupportedEncodingException {
        Map<String, String> map = new HashMap<>();
        for (String part : form.split("&")) {
            int i = part.indexOf('=');
            if (i >= 0) {
                String k = URLDecoder.decode(part.substring(0, i), "UTF-8");
                String v = URLDecoder.decode(part.substring(i + 1), "UTF-8");
                map.put(k, v);
            }
        }
        return map;
    }

    public static void initDatabase(String dbPath) {
        String url = "jdbc:sqlite:" + dbPath;
        String sql = "CREATE TABLE IF NOT EXISTS users (" +
                "username TEXT PRIMARY KEY, " +
                "password_hash TEXT NOT NULL, " +
                "salt TEXT NOT NULL)";
        try (Connection conn = DriverManager.getConnection(url);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("DB init error");
        }
    }

    private static boolean isValidUsername(String username) {
        return username != null && username.matches("^[A-Za-z0-9_]{3,50}$");
    }

    private static boolean isValidPassword(String pw) {
        if (pw == null || pw.length() < 8 || pw.length() > 200) return false;
        boolean hasLetter = false, hasDigit = false;
        for (char c : pw.toCharArray()) {
            if (Character.isLetter(c)) hasLetter = true;
            if (Character.isDigit(c)) hasDigit = true;
        }
        return hasLetter && hasDigit;
    }

    private static String base64(byte[] b) {
        return Base64.getEncoder().encodeToString(b);
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static byte[] randomBytes(int len) {
        byte[] r = new byte[len];
        new SecureRandom().nextBytes(r);
        return r;
    }

    public static String registerUser(String dbPath, String username, String password) {
        if (!isValidUsername(username)) return "ERR: invalid username";
        if (!isValidPassword(password)) return "ERR: weak password";

        String url = "jdbc:sqlite:" + dbPath;
        String insert = "INSERT INTO users(username, password_hash, salt) VALUES(?, ?, ?)";
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement ps = conn.prepareStatement(insert)) {
            byte[] salt = randomBytes(SALT_LEN);
            byte[] hash = pbkdf2(password.toCharArray(), salt);
            ps.setString(1, username);
            ps.setString(2, base64(hash));
            ps.setString(3, base64(salt));
            ps.executeUpdate();
            return "OK: registered";
        } catch (SQLException sqle) {
            if (sqle.getMessage() != null && sqle.getMessage().toLowerCase(Locale.ROOT).contains("unique")) {
                return "ERR: username exists";
            }
            return "ERR: database error";
        } catch (Exception e) {
            return "ERR: internal error";
        }
    }
}