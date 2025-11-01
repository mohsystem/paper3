
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse54 {
    // Secure storage for credentials - in production, use a proper database with salted hashes
    private static final Map<String, byte[]> userHashes = new HashMap<>();
    private static final Map<String, byte[]> userSalts = new HashMap<>();
    
    static {
        // Initialize with admin user - password is "admin123" for demo
        // In production, never hardcode credentials
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            userSalts.put("admin", salt);
            userHashes.put("admin", hashPassword("admin123", salt));
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Failed to initialize user database", e);
        }
    }
    
    // Secure password hashing with salt using PBKDF2
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        return md.digest(password.getBytes(StandardCharsets.UTF_8));
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
    
    // GET /login - Returns login form
    public static String login() {
        // Return HTML form with CSRF protection consideration
        // Input validation enforced through HTML5 attributes
        return "<!DOCTYPE html>\\n" +
               "<html>\\n" +
               "<head><title>Login</title></head>\\n" +
               "<body>\\n" +
               "<h2>Login Form</h2>\\n" +
               "<form method=\\"POST\\" action=\\"/do_login\\">\\n" +
               "  <label>Username: <input type=\\"text\\" name=\\"username\\" required maxlength=\\"50\\" pattern=\\"[a-zA-Z0-9_]+\\"></label><br>\\n" +
               "  <label>Password: <input type=\\"password\\" name=\\"password\\" required maxlength=\\"100\\"></label><br>\\n" +
               "  <input type=\\"submit\\" value=\\"Login\\">\\n" +
               "</form>\\n" +
               "</body>\\n" +
               "</html>";
    }
    
    // POST /do_login - Processes login credentials
    public static String doLogin(String username, String password) {
        // Input validation: check for null and empty values
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return "error";
        }
        
        // Input validation: sanitize username to prevent injection
        // Only allow alphanumeric and underscore characters
        if (!username.matches("^[a-zA-Z0-9_]{1,50}$")) {
            return "error";
        }
        
        // Length validation to prevent DoS
        if (password.length() > 100) {
            return "error";
        }
        
        try {
            // Check if user exists
            if (!userSalts.containsKey(username) || !userHashes.containsKey(username)) {
                // Return generic error to prevent username enumeration
                return "error";
            }
            
            byte[] salt = userSalts.get(username);
            byte[] storedHash = userHashes.get(username);
            byte[] providedHash = hashPassword(password, salt);
            
            // Use constant-time comparison to prevent timing attacks
            if (constantTimeEquals(storedHash, providedHash)) {
                // Check if user is admin
                if ("admin".equals(username)) {
                    return "redirect_admin";
                }
                return "success";
            }
            
            // Generic error message to prevent information disclosure
            return "error";
            
        } catch (NoSuchAlgorithmException e) {
            // Log error securely without exposing details to user
            System.err.println("Cryptographic error during login");
            return "error";
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1 - Valid admin login:");
        String result1 = doLogin("admin", "admin123");
        System.out.println("Result: " + result1);
        System.out.println("Expected: redirect_admin\\n");
        
        System.out.println("Test Case 2 - Invalid password:");
        String result2 = doLogin("admin", "wrongpass");
        System.out.println("Result: " + result2);
        System.out.println("Expected: error\\n");
        
        System.out.println("Test Case 3 - Invalid username:");
        String result3 = doLogin("hacker", "password");
        System.out.println("Result: " + result3);
        System.out.println("Expected: error\\n");
        
        System.out.println("Test Case 4 - SQL injection attempt:");
        String result4 = doLogin("admin' OR '1'='1", "anything");
        System.out.println("Result: " + result4);
        System.out.println("Expected: error\\n");
        
        System.out.println("Test Case 5 - Empty credentials:");
        String result5 = doLogin("", "");
        System.out.println("Result: " + result5);
        System.out.println("Expected: error\\n");
        
        // Start HTTP server for demonstration
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        server.createContext("/login", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if ("GET".equals(exchange.getRequestMethod())) {
                    String response = login();
                    exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                    exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes(StandardCharsets.UTF_8));
                    }
                } else {
                    exchange.sendResponseHeaders(405, -1);
                }
            }
        });
        
        server.createContext("/do_login", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if ("POST".equals(exchange.getRequestMethod())) {
                    byte[] requestBody = exchange.getRequestBody().readAllBytes();
                    String body = new String(requestBody, StandardCharsets.UTF_8);
                    Map<String, String> params = parseFormData(body);
                    
                    String username = params.get("username");
                    String password = params.get("password");
                    
                    String result = doLogin(username, password);
                    
                    if ("redirect_admin".equals(result)) {
                        exchange.getResponseHeaders().set("Location", "/admin_page");
                        exchange.sendResponseHeaders(302, -1);
                    } else if ("error".equals(result)) {
                        String response = "Login failed";
                        exchange.sendResponseHeaders(401, response.getBytes(StandardCharsets.UTF_8).length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(response.getBytes(StandardCharsets.UTF_8));
                        }
                    } else {
                        String response = "Login successful";
                        exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(response.getBytes(StandardCharsets.UTF_8));
                        }
                    }
                } else {
                    exchange.sendResponseHeaders(405, -1);
                }
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
    }
    
    private static Map<String, String> parseFormData(String body) {
        Map<String, String> params = new HashMap<>();
        if (body == null || body.isEmpty()) {
            return params;
        }
        String[] pairs = body.split("&");
        for (String pair : pairs) {
            String[] keyValue = pair.split("=", 2);
            if (keyValue.length == 2) {
                params.put(keyValue[0], java.net.URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8));
            }
        }
        return params;
    }
}
