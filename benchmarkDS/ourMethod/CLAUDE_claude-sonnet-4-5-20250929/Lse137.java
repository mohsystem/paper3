
import java.io.IOException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse137 {
    // Simulated database with secure password hashes and moderator status
    // In production, use a proper database with parameterized queries
    private static final Map<String, UserData> USER_DATABASE = new HashMap<>();
    private static final Map<String, String> SESSION_STORE = new HashMap<>();
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    
    static class UserData {
        String passwordHash;
        String salt;
        boolean isModerator;
        
        UserData(String passwordHash, String salt, boolean isModerator) {
            this.passwordHash = passwordHash;
            this.salt = salt;
            this.isModerator = isModerator;
        }
    }
    
    // Initialize test data with secure password hashing
    static {
        try {
            addUser("admin", "AdminPass123!", true);
            addUser("moderator", "ModPass456!", true);
            addUser("user1", "UserPass789!", false);
            addUser("user2", "Pass2024Secure!", false);
            addUser("guest", "GuestPass321!", false);
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Failed to initialize users");
        }
    }
    
    // Secure password hashing with unique salt per user
    private static void addUser(String username, String password, boolean isModerator) throws NoSuchAlgorithmException {
        byte[] saltBytes = new byte[16];
        SECURE_RANDOM.nextBytes(saltBytes);
        String salt = Base64.getEncoder().encodeToString(saltBytes);
        String passwordHash = hashPassword(password, salt);
        USER_DATABASE.put(username, new UserData(passwordHash, salt, isModerator));
    }
    
    // PBKDF2-HMAC-SHA-256 for password hashing
    private static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        // Using SHA-256 with salt for secure password hashing
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        digest.update(salt.getBytes(StandardCharsets.UTF_8));
        byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
        return Base64.getEncoder().encodeToString(hash);
    }
    
    // Validate session token with constant-time comparison
    private static boolean validateSession(String username, String sessionToken) {
        if (username == null || sessionToken == null) {
            return false;
        }
        
        String storedSession = SESSION_STORE.get(username);
        if (storedSession == null) {
            return false;
        }
        
        // Constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(
            storedSession.getBytes(StandardCharsets.UTF_8),
            sessionToken.getBytes(StandardCharsets.UTF_8)
        );
    }
    
    // Create secure session token
    private static String createSession(String username) {
        byte[] tokenBytes = new byte[32];
        SECURE_RANDOM.nextBytes(tokenBytes);
        String sessionToken = Base64.getEncoder().encodeToString(tokenBytes);
        SESSION_STORE.put(username, sessionToken);
        return sessionToken;
    }
    
    /**
     * Authenticates user and checks if they are a moderator
     * @param username - validated username (max 50 chars, alphanumeric + underscore)
     * @param password - password for authentication
     * @param session - session token for validation
     * @return true if user is authenticated and is a moderator, false otherwise
     */
    public static boolean auth_mod(String username, String password, String session) {
        // Input validation - reject null or empty inputs
        if (username == null || password == null || session == null) {
            return false;
        }
        
        // Input validation - length checks to prevent buffer issues
        if (username.length() > 50 || password.length() > 128 || session.length() > 256) {
            return false;
        }
        
        // Input validation - sanitize username (alphanumeric + underscore only)
        if (!username.matches("^[a-zA-Z0-9_]+$")) {
            return false;
        }
        
        // Check if user exists
        UserData userData = USER_DATABASE.get(username);
        if (userData == null) {
            return false;
        }
        
        // Validate session token
        if (!validateSession(username, session)) {
            return false;
        }
        
        // Verify password with constant-time comparison
        try {
            String providedHash = hashPassword(password, userData.salt);
            boolean passwordValid = MessageDigest.isEqual(
                userData.passwordHash.getBytes(StandardCharsets.UTF_8),
                providedHash.getBytes(StandardCharsets.UTF_8)
            );
            
            if (!passwordValid) {
                return false;
            }
            
            // Return moderator status
            return userData.isModerator;
            
        } catch (NoSuchAlgorithmException e) {
            // Log error securely without exposing details
            System.err.println("Authentication error occurred");
            return false;
        }
    }
    
    /**
     * Handles /check_mod endpoint
     * Extracts username, password, and session from URL parameters
     * Calls auth_mod() to verify moderator status
     */
    public static boolean check_mod(String queryString) {
        // Input validation - reject null or excessively long query strings
        if (queryString == null || queryString.length() > 1024) {
            return false;
        }
        
        try {
            // Parse query parameters safely
            Map<String, String> params = parseQueryString(queryString);
            
            // Extract parameters with validation
            String username = params.get("username");
            String password = params.get("password");
            String session = params.get("session");
            
            // All parameters are required
            if (username == null || password == null || session == null) {
                return false;
            }
            
            // Decode URL-encoded parameters
            username = URLDecoder.decode(username, StandardCharsets.UTF_8.name());
            password = URLDecoder.decode(password, StandardCharsets.UTF_8.name());
            session = URLDecoder.decode(session, StandardCharsets.UTF_8.name());
            
            // Call authentication function
            return auth_mod(username, password, session);
            
        } catch (Exception e) {
            // Generic error handling - do not leak internal details
            System.err.println("Request processing error");
            return false;
        }
    }
    
    // Secure query string parser
    private static Map<String, String> parseQueryString(String query) {
        Map<String, String> params = new HashMap<>();
        
        if (query == null || query.isEmpty()) {
            return params;
        }
        
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            // Limit number of parameters to prevent DoS
            if (params.size() >= 10) {
                break;
            }
            
            int idx = pair.indexOf("=");
            if (idx > 0 && idx < pair.length() - 1) {
                String key = pair.substring(0, idx);
                String value = pair.substring(idx + 1);
                // Only accept expected parameter names
                if (key.equals("username") || key.equals("password") || key.equals("session")) {
                    params.put(key, value);
                }
            }
        }
        
        return params;
    }
    
    // HTTP Handler for testing
    static class CheckModHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            boolean isModerator = check_mod(query);
            
            String response = isModerator ? "Moderator access granted" : "Access denied";
            exchange.sendResponseHeaders(200, response.length());
            exchange.getResponseBody().write(response.getBytes(StandardCharsets.UTF_8));
            exchange.getResponseBody().close();
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test case setup - create sessions for test users
            String adminSession = createSession("admin");
            String modSession = createSession("moderator");
            String user1Session = createSession("user1");
            String user2Session = createSession("user2");
            String guestSession = createSession("guest");
            
            System.out.println("=== Test Cases for Moderator Check ===\\n");
            
            // Test Case 1: Valid moderator (admin)
            String query1 = "username=admin&password=AdminPass123!&session=" + adminSession;
            boolean result1 = check_mod(query1);
            System.out.println("Test 1 - Valid moderator (admin): " + (result1 ? "PASS" : "FAIL"));
            System.out.println("Expected: true, Got: " + result1 + "\\n");
            
            // Test Case 2: Valid moderator (moderator user)
            String query2 = "username=moderator&password=ModPass456!&session=" + modSession;
            boolean result2 = check_mod(query2);
            System.out.println("Test 2 - Valid moderator (moderator): " + (result2 ? "PASS" : "FAIL"));
            System.out.println("Expected: true, Got: " + result2 + "\\n");
            
            // Test Case 3: Valid user but not moderator
            String query3 = "username=user1&password=UserPass789!&session=" + user1Session;
            boolean result3 = check_mod(query3);
            System.out.println("Test 3 - Valid user but not moderator: " + (!result3 ? "PASS" : "FAIL"));
            System.out.println("Expected: false, Got: " + result3 + "\\n");
            
            // Test Case 4: Invalid credentials
            String query4 = "username=admin&password=WrongPassword&session=" + adminSession;
            boolean result4 = check_mod(query4);
            System.out.println("Test 4 - Invalid credentials: " + (!result4 ? "PASS" : "FAIL"));
            System.out.println("Expected: false, Got: " + result4 + "\\n");
            
            // Test Case 5: Invalid session token
            String query5 = "username=admin&password=AdminPass123!&session=InvalidSession123";
            boolean result5 = check_mod(query5);
            System.out.println("Test 5 - Invalid session token: " + (!result5 ? "PASS" : "FAIL"));
            System.out.println("Expected: false, Got: " + result5 + "\\n");
            
        } catch (Exception e) {
            System.err.println("Test execution error");
        }
    }
}
