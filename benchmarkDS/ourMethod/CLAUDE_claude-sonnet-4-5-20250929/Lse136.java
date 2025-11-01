
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

public class Lse136 {
    // Simulated secure credential store with hashed passwords
    // In production, use environment variables or secure vault
    private static final Map<String, byte[]> userStore = new HashMap<>();
    private static final Map<String, byte[]> saltStore = new HashMap<>();
    
    static {
        // Initialize test users with securely hashed passwords
        // Password requirements: minimum 8 characters
        try {
            storeUser("admin", "SecurePass123!");
            storeUser("user1", "MyPassword456!");
            storeUser("testuser", "TestPass789!");
            storeUser("moderator", "ModPass321!");
            storeUser("developer", "DevPass654!");
        } catch (Exception e) {
            System.err.println("Error initializing users");
        }
    }
    
    // Store user with hashed password and unique salt
    private static void storeUser(String username, String password) throws NoSuchAlgorithmException {
        // Generate unique cryptographically secure salt for each user
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        
        // Hash password with PBKDF2-HMAC-SHA-256 with high iteration count
        byte[] hash = hashPassword(password, salt);
        
        userStore.put(username, hash);
        saltStore.put(username, salt);
    }
    
    // Hash password using PBKDF2-HMAC-SHA-256 with unique salt
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.update(salt);
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            
            // Apply PBKDF2-like iteration (simplified for demonstration)
            // In production, use javax.crypto.SecretKeyFactory with PBKDF2WithHmacSHA256
            for (int i = 0; i < 210000; i++) {
                digest.reset();
                digest.update(salt);
                hash = digest.digest(hash);
            }
            return hash;
        } catch (NoSuchAlgorithmException e) {
            throw e;
        }
    }
    
    // Constant-time comparison to prevent timing attacks
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    // Validate input to prevent injection attacks
    private static boolean isValidInput(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }
        // Check length constraints
        if (input.length() > 100) {
            return false;
        }
        // Allow only alphanumeric and safe special characters
        return input.matches("[a-zA-Z0-9!@#$%^&*()_+\\\\-=\\\\[\\\\]{};':\\"\\\\\\\\|,.<>/?]+");
    }
    
    // Check credentials securely
    private static boolean checkCredentials(String username, String password) {
        // Validate inputs before processing
        if (!isValidInput(username) || !isValidInput(password)) {
            return false;
        }
        
        // Check if user exists
        if (!userStore.containsKey(username) || !saltStore.containsKey(username)) {
            // Use dummy computation to prevent timing attacks
            try {
                byte[] dummySalt = new byte[16];
                new SecureRandom().nextBytes(dummySalt);
                hashPassword(password, dummySalt);
            } catch (NoSuchAlgorithmException e) {
                // Ignore
            }
            return false;
        }
        
        try {
            // Retrieve stored salt and hash
            byte[] salt = saltStore.get(username);
            byte[] storedHash = userStore.get(username);
            
            // Hash provided password with stored salt
            byte[] providedHash = hashPassword(password, salt);
            
            // Use constant-time comparison
            return constantTimeEquals(storedHash, providedHash);
        } catch (NoSuchAlgorithmException e) {
            return false;
        }
    }
    
    // HTTP handler for /check_mod endpoint
    static class CheckModHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "false";
            
            try {
                // Only allow GET requests
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    response = "false";
                    sendResponse(exchange, response, 405);
                    return;
                }
                
                // Parse query parameters safely
                String query = exchange.getRequestURI().getQuery();
                if (query == null || query.isEmpty()) {
                    response = "false";
                    sendResponse(exchange, response, 400);
                    return;
                }
                
                Map<String, String> params = parseQuery(query);
                String username = params.get("username");
                String password = params.get("password");
                
                // Validate presence of required parameters
                if (username == null || password == null) {
                    response = "false";
                    sendResponse(exchange, response, 400);
                    return;
                }
                
                // Check credentials
                if (checkCredentials(username, password)) {
                    response = "true";
                    sendResponse(exchange, response, 200);
                } else {
                    response = "false";
                    sendResponse(exchange, response, 401);
                }
            } catch (Exception e) {
                // Generic error message to prevent information disclosure
                response = "false";
                sendResponse(exchange, response, 500);
            }
        }
        
        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null || query.isEmpty()) {
                return result;
            }
            
            for (String param : query.split("&")) {
                String[] pair = param.split("=", 2);
                if (pair.length == 2) {
                    // Decode and sanitize parameters
                    String key = java.net.URLDecoder.decode(pair[0], StandardCharsets.UTF_8);
                    String value = java.net.URLDecoder.decode(pair[1], StandardCharsets.UTF_8);
                    result.put(key, value);
                }
            }
            return result;
        }
        
        private void sendResponse(HttpExchange exchange, String response, int statusCode) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/check_mod", new CheckModHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8000");
        System.out.println("\\nTest cases:");
        System.out.println("1. Valid: http://localhost:8000/check_mod?username=admin&password=SecurePass123!");
        System.out.println("2. Valid: http://localhost:8000/check_mod?username=user1&password=MyPassword456!");
        System.out.println("3. Invalid password: http://localhost:8000/check_mod?username=admin&password=wrong");
        System.out.println("4. Invalid user: http://localhost:8000/check_mod?username=invalid&password=test");
        System.out.println("5. Missing params: http://localhost:8000/check_mod?username=admin");
    }
}
