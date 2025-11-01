// Chain-of-Through process:
// 1) Problem understanding: Build a minimal HTTP server for user registration, accept username/password, validate them, hash password with a salt, and store in an in-memory database. Provide a function that accepts parameters and returns output. Include a main with 5 test cases.
// 2) Security requirements: Use strong password hashing (PBKDF2), per-user random salt, input validation, size limits for requests, avoid logging secrets, thread-safe data structure.
// 3) Secure coding generation: Implement PBKDF2WithHmacSHA256, salt generation via SecureRandom, prepared response handling, and validation.
// 4) Code review: Ensure no sensitive data leaks, proper clearing of password arrays, bounded inputs, safe concurrency, and minimal attack surface.
// 5) Secure code output: Final code below applies mitigations and provides tests.

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse91 {
    // In-memory "database": username -> "base64(salt):base64(hash)"
    private static final Map<String, String> DB = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");
    private static final int MIN_PASSWORD_LEN = 8;
    private static final int MAX_PASSWORD_LEN = 1024;
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 200_000;
    private static final int KEY_LENGTH_BITS = 256;

    public static String registerUser(String username, char[] password) {
        if (username == null || password == null) {
            zeroCharArray(password);
            return "error: invalid input";
        }
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            zeroCharArray(password);
            return "error: invalid username";
        }
        if (password.length < MIN_PASSWORD_LEN || password.length > MAX_PASSWORD_LEN) {
            zeroCharArray(password);
            return "error: invalid password length";
        }
        if (DB.containsKey(username)) {
            zeroCharArray(password);
            return "error: username already exists";
        }
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        String record;
        try {
            String hashed = hashPassword(password, salt, ITERATIONS, KEY_LENGTH_BITS);
            record = hashed;
        } catch (Exception e) {
            zeroCharArray(password);
            return "error: hashing failed";
        }
        zeroCharArray(password);
        DB.put(username, record);
        return "ok";
    }

    private static String hashPassword(char[] password, byte[] salt, int iterations, int keyLength) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLength);
        SecretKeyFactory skf;
        try {
            skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        } catch (NoSuchAlgorithmException e) {
            skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
        }
        byte[] hash = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        String saltB64 = Base64.getEncoder().encodeToString(salt);
        String hashB64 = Base64.getEncoder().encodeToString(hash);
        return saltB64 + ":" + hashB64;
    }

    private static void zeroCharArray(char[] arr) {
        if (arr != null) {
            for (int i = 0; i < arr.length; i++) arr[i] = 0;
        }
    }

    // Minimal HTTP server for POST /register with application/x-www-form-urlencoded
    private static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/register", new RegisterHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
    }

    private static class RegisterHandler implements HttpHandler {
        private static final int MAX_BODY_SIZE = 4096;

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, "{\"status\":\"error\",\"message\":\"method not allowed\"}");
                    return;
                }
                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                if (contentType == null || !contentType.toLowerCase().contains("application/x-www-form-urlencoded")) {
                    sendJson(exchange, 400, "{\"status\":\"error\",\"message\":\"unsupported content type\"}");
                    return;
                }
                int contentLen = 0;
                try {
                    contentLen = Integer.parseInt(exchange.getRequestHeaders().getFirst("Content-Length"));
                } catch (Exception ignored) {}
                if (contentLen <= 0 || contentLen > MAX_BODY_SIZE) {
                    sendJson(exchange, 400, "{\"status\":\"error\",\"message\":\"invalid content length\"}");
                    return;
                }
                byte[] body = readAll(exchange.getRequestBody(), MAX_BODY_SIZE);
                String payload = new String(body, StandardCharsets.UTF_8);
                String[] parts = payload.split("&");
                String username = null;
                String passwordStr = null;
                for (String p : parts) {
                    String[] kv = p.split("=", 2);
                    if (kv.length == 2) {
                        String key = URLDecoder.decode(kv[0], StandardCharsets.UTF_8);
                        String val = URLDecoder.decode(kv[1], StandardCharsets.UTF_8);
                        if ("username".equals(key)) username = val;
                        else if ("password".equals(key)) passwordStr = val;
                    }
                }
                if (username == null || passwordStr == null) {
                    sendJson(exchange, 400, "{\"status\":\"error\",\"message\":\"missing fields\"}");
                    return;
                }
                char[] pwdChars = passwordStr.toCharArray();
                String result = registerUser(username, pwdChars);
                if ("ok".equals(result)) {
                    sendJson(exchange, 200, "{\"status\":\"ok\"}");
                } else {
                    sendJson(exchange, 400, "{\"status\":\"error\",\"message\":\"" + safeJson(result) + "\"}");
                }
            } catch (Exception e) {
                sendJson(exchange, 500, "{\"status\":\"error\",\"message\":\"server error\"}");
            }
        }

        private byte[] readAll(InputStream is, int max) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[1024];
            int total = 0;
            int r;
            while ((r = is.read(buf)) != -1) {
                total += r;
                if (total > max) throw new IOException("too large");
                baos.write(buf, 0, r);
            }
            return baos.toByteArray();
        }

        private void sendJson(HttpExchange ex, int code, String json) throws IOException {
            byte[] data = json.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }

        private String safeJson(String s) {
            return s.replace("\\", "\\\\").replace("\"", "\\\"");
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        System.out.println(registerUser("user_one", "StrongPass123!".toCharArray())); // ok
        System.out.println(registerUser("user_one", "AnotherPass123!".toCharArray())); // duplicate
        System.out.println(registerUser("u!", "bad".toCharArray())); // invalid username
        System.out.println(registerUser("shortpwd", "123".toCharArray())); // short password
        System.out.println(registerUser("second_user", "Another#Strong$Pass456".toCharArray())); // ok

        // Start server on port 8080 (optional for manual testing)
        // Access: curl -X POST -d "username=tester&password=TestPass123!" http://localhost:8080/register
        startServer(8080);
    }
}