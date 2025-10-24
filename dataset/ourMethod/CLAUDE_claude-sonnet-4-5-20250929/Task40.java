
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task40 {
    private static final Map<String, String> sessionTokens = new ConcurrentHashMap<>();
    private static final Map<String, UserSettings> userSettings = new ConcurrentHashMap<>();
    private static final byte[] SECRET_KEY = new byte[32];
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_EMAIL_LENGTH = 100;
    private static final int MAX_THEME_LENGTH = 20;

    static {
        SecureRandom secureRandom = new SecureRandom();
        secureRandom.nextBytes(SECRET_KEY);
    }

    static class UserSettings {
        String username;
        String email;
        String theme;

        UserSettings(String username, String email, String theme) {
            this.username = username;
            this.email = email;
            this.theme = theme;
        }
    }

    public static String generateCSRFToken(String sessionId) {
        try {
            SecureRandom random = new SecureRandom();
            byte[] tokenBytes = new byte[32];
            random.nextBytes(tokenBytes);
            String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
            
            Mac hmac = Mac.getInstance("HmacSHA256");
            SecretKeySpec keySpec = new SecretKeySpec(SECRET_KEY, "HmacSHA256");
            hmac.init(keySpec);
            byte[] signatureBytes = hmac.doFinal((sessionId + ":" + token).getBytes(StandardCharsets.UTF_8));
            String signature = Base64.getUrlEncoder().withoutPadding().encodeToString(signatureBytes);
            
            String csrfToken = token + "." + signature;
            sessionTokens.put(sessionId, csrfToken);
            return csrfToken;
        } catch (NoSuchAlgorithmException | InvalidKeyException e) {
            throw new RuntimeException("Failed to generate CSRF token", e);
        }
    }

    public static boolean validateCSRFToken(String sessionId, String token) {
        if (sessionId == null || token == null || sessionId.isEmpty() || token.isEmpty()) {
            return false;
        }
        
        String storedToken = sessionTokens.get(sessionId);
        if (storedToken == null) {
            return false;
        }

        return constantTimeEquals(storedToken, token);
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        byte[] aBytes = a.getBytes(StandardCharsets.UTF_8);
        byte[] bBytes = b.getBytes(StandardCharsets.UTF_8);
        
        if (aBytes.length != bBytes.length) {
            return false;
        }
        
        int result = 0;
        for (int i = 0; i < aBytes.length; i++) {
            result |= aBytes[i] ^ bBytes[i];
        }
        return result == 0;
    }

    public static String sanitizeInput(String input, int maxLength) {
        if (input == null) {
            return "";
        }
        input = input.trim();
        if (input.length() > maxLength) {
            input = input.substring(0, maxLength);
        }
        return input.replaceAll("[<>\\"'&]", "");
    }

    public static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        String emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$";
        return email.matches(emailRegex) && email.length() <= MAX_EMAIL_LENGTH;
    }

    public static String updateUserSettings(String sessionId, String csrfToken, 
                                          String username, String email, String theme) {
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "Error: Invalid CSRF token";
        }

        username = sanitizeInput(username, MAX_USERNAME_LENGTH);
        email = sanitizeInput(email, MAX_EMAIL_LENGTH);
        theme = sanitizeInput(theme, MAX_THEME_LENGTH);

        if (username.isEmpty()) {
            return "Error: Username cannot be empty";
        }

        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }

        if (!theme.matches("^(light|dark|auto)$")) {
            return "Error: Invalid theme selection";
        }

        UserSettings settings = new UserSettings(username, email, theme);
        userSettings.put(sessionId, settings);

        sessionTokens.remove(sessionId);

        return "Success: Settings updated successfully";
    }

    static class FormHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = generateSessionId();
            String csrfToken = generateCSRFToken(sessionId);

            if ("GET".equals(exchange.getRequestMethod())) {
                String response = buildForm(sessionId, csrfToken);
                sendResponse(exchange, 200, response, "text/html");
            } else if ("POST".equals(exchange.getRequestMethod())) {
                Map<String, String> params = parseFormData(exchange);
                String receivedSessionId = params.getOrDefault("sessionId", "");
                String receivedCsrfToken = params.getOrDefault("csrf_token", "");
                String username = params.getOrDefault("username", "");
                String email = params.getOrDefault("email", "");
                String theme = params.getOrDefault("theme", "");

                String result = updateUserSettings(receivedSessionId, receivedCsrfToken, 
                                                  username, email, theme);
                sendResponse(exchange, 200, result, "text/plain");
            } else {
                sendResponse(exchange, 405, "Method Not Allowed", "text/plain");
            }
        }

        private String buildForm(String sessionId, String csrfToken) {
            return "<!DOCTYPE html><html><head><title>User Settings</title></head><body>" +
                   "<h2>Update User Settings</h2>" +
                   "<form method='POST' action='/'>" +
                   "<input type='hidden' name='sessionId' value='" + escapeHtml(sessionId) + "'>" +
                   "<input type='hidden' name='csrf_token' value='" + escapeHtml(csrfToken) + "'>" +
                   "<label>Username: <input type='text' name='username' maxlength='50' required></label><br>" +
                   "<label>Email: <input type='email' name='email' maxlength='100' required></label><br>" +
                   "<label>Theme: <select name='theme'><option value='light'>Light</option>" +
                   "<option value='dark'>Dark</option><option value='auto'>Auto</option></select></label><br>" +
                   "<button type='submit'>Update Settings</button>" +
                   "</form></body></html>";
        }

        private String escapeHtml(String input) {
            return input.replace("&", "&amp;")
                       .replace("<", "&lt;")
                       .replace(">", "&gt;")
                       .replace("\\"", "&quot;")
                       .replace("'", "&#x27;");
        }

        private Map<String, String> parseFormData(HttpExchange exchange) throws IOException {
            Map<String, String> params = new HashMap<>();
            byte[] buffer = new byte[8192];
            int bytesRead = exchange.getRequestBody().read(buffer);
            if (bytesRead > 0) {
                String body = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
                String[] pairs = body.split("&");
                for (String pair : pairs) {
                    String[] keyValue = pair.split("=", 2);
                    if (keyValue.length == 2) {
                        String key = URLDecoder.decode(keyValue[0], StandardCharsets.UTF_8);
                        String value = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8);
                        params.put(key, value);
                    }
                }
            }
            return params;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response, 
                                 String contentType) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", contentType + "; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }

        private String generateSessionId() {
            SecureRandom random = new SecureRandom();
            byte[] sessionBytes = new byte[32];
            random.nextBytes(sessionBytes);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(sessionBytes);
        }
    }

    public static void main(String[] args) throws IOException {
        System.out.println("Test Case 1: Valid CSRF token");
        String session1 = "session1";
        String token1 = generateCSRFToken(session1);
        String result1 = updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark");
        System.out.println(result1);

        System.out.println("\\nTest Case 2: Invalid CSRF token");
        String session2 = "session2";
        generateCSRFToken(session2);
        String result2 = updateUserSettings(session2, "invalid_token", "jane_doe", "jane@example.com", "light");
        System.out.println(result2);

        System.out.println("\\nTest Case 3: Invalid email format");
        String session3 = "session3";
        String token3 = generateCSRFToken(session3);
        String result3 = updateUserSettings(session3, token3, "bob", "invalid-email", "auto");
        System.out.println(result3);

        System.out.println("\\nTest Case 4: Empty username");
        String session4 = "session4";
        String token4 = generateCSRFToken(session4);
        String result4 = updateUserSettings(session4, token4, "", "alice@example.com", "light");
        System.out.println(result4);

        System.out.println("\\nTest Case 5: Invalid theme");
        String session5 = "session5";
        String token5 = generateCSRFToken(session5);
        String result5 = updateUserSettings(session5, token5, "charlie", "charlie@example.com", "invalid_theme");
        System.out.println(result5);

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new FormHandler());
        server.setExecutor(null);
        System.out.println("\\nServer started on port 8080");
        server.start();
    }
}
