import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class Lse54 {

    // NOTE: In a real application, NEVER hardcode credentials.
    // Use a secure password hashing mechanism (e.g., Argon2, bcrypt) and store hashes in a database.
    private static final String ADMIN_USERNAME = "admin";
    private static final byte[] ADMIN_PASSWORD_BYTES = "SuperSecurePassword123!".getBytes(StandardCharsets.UTF_8);

    // In-memory stores for session and CSRF tokens. In production, use a persistent, scalable solution.
    private static final Map<String, String> sessions = new ConcurrentHashMap<>(); // SessionID -> Username
    private static final Map<String, String> csrfTokens = new ConcurrentHashMap<>(); // SessionID -> CSRF Token

    private static final SecureRandom secureRandom = new SecureRandom();

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/login", new LoginHandler());
        server.createContext("/do_login", new DoLoginHandler());
        server.createContext("/admin_page", new AdminPageHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- How to run test cases ---");
        System.out.println("1. Access Login Page: Open a browser and go to http://localhost:8080/login");
        System.out.println("2. Successful Login: Use username 'admin' and password 'SuperSecurePassword123!'");
        System.out.println("3. Failed Login (Wrong Pass): Use username 'admin' and any other password.");
        System.out.println("4. Failed Login (Wrong User): Use any username other than 'admin'.");
        System.out.println("5. Unauthorized Access: Try to directly access http://localhost:8080/admin_page in a new private/incognito browser window.");
    }

    private static String getSessionIdFromCookie(HttpExchange exchange) {
        String cookieHeader = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookieHeader != null) {
            for (String cookie : cookieHeader.split(";")) {
                String[] parts = cookie.trim().split("=", 2);
                if (parts.length == 2 && "sessionId".equals(parts[0])) {
                    return parts[1];
                }
            }
        }
        return null;
    }
    
    private static String generateRandomToken() {
        byte[] bytes = new byte[32];
        secureRandom.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> map = new HashMap<>();
        if (formData == null || formData.isEmpty()) {
            return map;
        }
        for (String param : formData.split("&")) {
            String[] pair = param.split("=", 2);
            if (pair.length == 2) {
                try {
                    String key = URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name());
                    String value = URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name());
                    map.put(key, value);
                } catch (java.io.UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                }
            }
        }
        return map;
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionIdFromCookie(exchange);
            if (sessionId != null && sessions.containsKey(sessionId)) {
                exchange.getResponseHeaders().set("Location", "/admin_page");
                exchange.sendResponseHeaders(302, -1);
                return;
            }
            
            String newSessionId = UUID.randomUUID().toString();
            String csrfToken = generateRandomToken();
            csrfTokens.put(newSessionId, csrfToken);

            String response = getLoginFormHTML(csrfToken, null);

            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
            exchange.getResponseHeaders().set("Set-Cookie", "sessionId=" + newSessionId + "; HttpOnly; SameSite=Strict");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
    
    private static String getLoginFormHTML(String csrfToken, String error) {
        return "<!DOCTYPE html>" +
                "<html>" +
                "<head><title>Login</title></head>" +
                "<body>" +
                "<h2>Login</h2>" +
                (error != null ? "<p style='color:red;'>" + error + "</p>" : "") +
                "<form action='/do_login' method='post'>" +
                "<label for='username'>Username:</label><br>" +
                "<input type='text' id='username' name='username'><br>" +
                "<label for='password'>Password:</label><br>" +
                "<input type='password' id='password' name='password'><br><br>" +
                "<input type='hidden' name='csrf_token' value='" + csrfToken + "'>" +
                "<input type='submit' value='Login'>" +
                "</form>" +
                "</body>" +
                "</html>";
    }

    static class DoLoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
                return;
            }

            String sessionId = getSessionIdFromCookie(exchange);
            if (sessionId == null || !csrfTokens.containsKey(sessionId)) {
                sendUnauthorized(exchange, "Invalid session or CSRF token missing.");
                return;
            }

            String body;
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))) {
                body = reader.lines().collect(Collectors.joining("\n"));
            }
            Map<String, String> params = parseFormData(body);
            String username = params.getOrDefault("username", "");
            String password = params.getOrDefault("password", "");
            String formCsrfToken = params.getOrDefault("csrf_token", "");

            String expectedCsrfToken = csrfTokens.remove(sessionId); // Use token only once
            if (expectedCsrfToken == null || !MessageDigest.isEqual(expectedCsrfToken.getBytes(StandardCharsets.UTF_8), formCsrfToken.getBytes(StandardCharsets.UTF_8))) {
                sendUnauthorized(exchange, "CSRF token validation failed.");
                return;
            }

            boolean isUsernameMatch = ADMIN_USERNAME.equals(username);
            boolean isPasswordMatch = MessageDigest.isEqual(ADMIN_PASSWORD_BYTES, password.getBytes(StandardCharsets.UTF_8));

            if (isUsernameMatch && isPasswordMatch) {
                sessions.put(sessionId, username);
                exchange.getResponseHeaders().set("Location", "/admin_page");
                exchange.sendResponseHeaders(302, -1);
            } else {
                String newCsrfToken = generateRandomToken();
                csrfTokens.put(sessionId, newCsrfToken);
                String response = getLoginFormHTML(newCsrfToken, "Invalid username or password.");
                
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
                exchange.sendResponseHeaders(401, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }
    
    private static void sendUnauthorized(HttpExchange exchange, String message) throws IOException {
        byte[] response = message.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        exchange.sendResponseHeaders(403, response.length);
        try(OutputStream os = exchange.getResponseBody()) {
            os.write(response);
        }
    }

    static class AdminPageHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionIdFromCookie(exchange);
            if (sessionId != null && ADMIN_USERNAME.equals(sessions.get(sessionId))) {
                String response = "<!DOCTYPE html><html><head><title>Admin Page</title></head>" +
                                  "<body><h1>Welcome, Admin!</h1></body></html>";
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                exchange.getResponseHeaders().set("Location", "/login");
                exchange.sendResponseHeaders(302, -1);
            }
        }
    }
}