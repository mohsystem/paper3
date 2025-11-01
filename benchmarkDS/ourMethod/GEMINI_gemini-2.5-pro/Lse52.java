import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Lse52 {

    // --- User Store (In-memory for demonstration) ---
    // Rule #7: Avoid hardcoding credentials. This is for demonstration only.
    // In a real application, use a secure database and manage credentials properly.
    private static final Map<String, UserInfo> userDatabase = new ConcurrentHashMap<>();

    // --- Session Store (In-memory for demonstration) ---
    private static final Map<String, String> sessionStore = new ConcurrentHashMap<>();

    // Record to hold user's salt and hashed password
    private record UserInfo(byte[] salt, String hashedPassword) {}

    // --- Password Hashing Utilities ---
    // Rule #5, #8, #9: Use strong hashing with a unique, cryptographically random salt.
    private static final SecureRandom random = new SecureRandom();
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 128;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    private static byte[] generateSalt() {
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private static String hashPassword(String password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // This should not happen with standard algorithms
            throw new RuntimeException("Error hashing password", e);
        }
    }

    private static boolean verifyPassword(String providedPassword, String storedHash, byte[] salt) {
        String newHash = hashPassword(providedPassword, salt);
        return newHash.equals(storedHash);
    }
    
    // Helper to add a user to the "database"
    private static void addUser(String username, String password) {
        byte[] salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, new UserInfo(salt, hashedPassword));
    }


    public static void main(String[] args) throws IOException {
        // --- Setup Users for Test Cases ---
        addUser("admin", "password123");
        addUser("testuser", "SecurePass!1");
        addUser("user1", "Pass@word1");
        addUser("user2", "Anotherpwd");
        addUser("user3", "Pa$$w0rd");
        
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new IndexHandler());
        server.createContext("/login", new LoginHandler());
        server.createContext("/secret", new SecretHandler());
        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("Navigate to http://127.0.0.1:" + port);
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. Visit http://127.0.0.1:8080/ - Should show the index page.");
        System.out.println("2. Click the login link - Should show the login form.");
        System.out.println("3. Login with incorrect credentials (e.g., admin/wrong) - Should show an error.");
        System.out.println("4. Login with correct credentials (e.g., admin/password123) - Should redirect to /secret.");
        System.out.println("5. Visit http://127.0.0.1:8080/secret directly without logging in - Should redirect to /login.");
    }

    // --- HTTP Handlers ---
    static class IndexHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<!DOCTYPE html><html><head><title>Index</title></head>"
                            + "<body><h1>Welcome</h1><p><a href=\"/login\">Please login</a></p></body></html>";
            sendResponse(exchange, 200, response);
        }
    }

    static class SecretHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionIdFromCookie(exchange);
            if (sessionId != null && sessionStore.containsKey(sessionId)) {
                String username = sessionStore.get(sessionId);
                // Rule #10: Escape user-controlled data to prevent XSS
                String safeUsername = htmlEscape(username);
                String response = "<!DOCTYPE html><html><head><title>Secret</title></head>"
                                + "<body><h1>Secret settings</h1><p>Welcome, " + safeUsername + "!</p></body></html>";
                sendResponse(exchange, 200, response);
            } else {
                // Redirect to login
                exchange.getResponseHeaders().set("Location", "/login");
                exchange.sendResponseHeaders(302, -1);
            }
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                serveLoginForm(exchange, null);
            } else if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                handleLoginAttempt(exchange);
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
            }
        }
        
        private void serveLoginForm(HttpExchange exchange, String error) throws IOException {
            String errorHtml = (error != null) ? "<p style=\"color:red;\">" + htmlEscape(error) + "</p>" : "";
            String response = "<!DOCTYPE html><html><head><title>Login</title></head>"
                            + "<body><h1>Login</h1>" + errorHtml
                            + "<form method=\"post\">"
                            + "<label for=\"username\">Username:</label>"
                            + "<input type=\"text\" id=\"username\" name=\"username\" required><br><br>"
                            + "<label for=\"password\">Password:</label>"
                            + "<input type=\"password\" id=\"password\" name=\"password\" required><br><br>"
                            + "<input type=\"submit\" value=\"Login\">"
                            + "</form></body></html>";
            sendResponse(exchange, 200, response);
        }

        private void handleLoginAttempt(HttpExchange exchange) throws IOException {
            String body = new String(exchange.getRequestBody().readAllBytes(), StandardCharsets.UTF_8);
            Map<String, String> params = parseFormData(body);
            // Rule #1: Validate inputs
            String username = params.get("username");
            String password = params.get("password");

            if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
                serveLoginForm(exchange, "Username and password are required.");
                return;
            }

            UserInfo user = userDatabase.get(username);
            if (user != null && verifyPassword(password, user.hashedPassword(), user.salt())) {
                // Correct credentials
                String sessionId = UUID.randomUUID().toString();
                sessionStore.put(sessionId, username);
                // Set a secure cookie (in a real app, add ; Secure; HttpOnly; SameSite=Strict)
                exchange.getResponseHeaders().set("Set-Cookie", "sessionId=" + sessionId + "; Path=/");
                exchange.getResponseHeaders().set("Location", "/secret");
                exchange.sendResponseHeaders(302, -1);
            } else {
                // Invalid credentials
                serveLoginForm(exchange, "Invalid username or password.");
            }
        }
    }

    // --- Helper Utilities ---
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(statusCode, responseBytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(responseBytes);
        }
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> map = new HashMap<>();
        if (formData == null || formData.isEmpty()) return map;
        return Stream.of(formData.split("&"))
                     .map(s -> s.split("=", 2))
                     .filter(pair -> pair.length == 2)
                     .collect(Collectors.toMap(
                         pair -> URLDecoder.decode(pair[0], StandardCharsets.UTF_8),
                         pair -> URLDecoder.decode(pair[1], StandardCharsets.UTF_8),
                         (v1, v2) -> v1 // handle duplicate keys
                     ));
    }
    
    private static String getSessionIdFromCookie(HttpExchange exchange) {
        String cookieHeader = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookieHeader != null) {
            return Stream.of(cookieHeader.split(";"))
                         .map(String::trim)
                         .map(s -> s.split("=", 2))
                         .filter(pair -> pair.length == 2 && "sessionId".equals(pair[0]))
                         .map(pair -> pair[1])
                         .findFirst()
                         .orElse(null);
        }
        return null;
    }

    private static String htmlEscape(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#x27;");
    }
}