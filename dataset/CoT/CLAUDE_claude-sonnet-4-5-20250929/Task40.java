
// Java Implementation using Spring Boot framework concepts
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

class Task40 {
    private static Map<String, String> sessionTokens = new HashMap<>();
    private static Map<String, UserSettings> userDatabase = new HashMap<>();
    
    static class UserSettings {
        private String username;
        private String email;
        private String theme;
        
        public UserSettings(String username, String email, String theme) {
            this.username = username;
            this.email = email;
            this.theme = theme;
        }
        
        @Override
        public String toString() {
            return "UserSettings{username='" + username + "', email='" + email + "', theme='" + theme + "'}";
        }
    }
    
    static class CSRFToken {
        private String token;
        private long timestamp;
        private static final long TOKEN_VALIDITY = 3600000; // 1 hour
        
        public CSRFToken() {
            this.token = generateSecureToken();
            this.timestamp = System.currentTimeMillis();
        }
        
        private String generateSecureToken() {
            try {
                SecureRandom random = SecureRandom.getInstanceStrong();
                byte[] bytes = new byte[32];
                random.nextBytes(bytes);
                return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
            } catch (Exception e) {
                return UUID.randomUUID().toString();
            }
        }
        
        public String getToken() {
            return token;
        }
        
        public boolean isValid() {
            return (System.currentTimeMillis() - timestamp) < TOKEN_VALIDITY;
        }
    }
    
    public static String generateCSRFToken(String sessionId) {
        if (sessionId == null || sessionId.isEmpty()) {
            throw new IllegalArgumentException("Session ID cannot be null or empty");
        }
        
        CSRFToken csrfToken = new CSRFToken();
        sessionTokens.put(sessionId, csrfToken.getToken());
        return csrfToken.getToken();
    }
    
    public static boolean validateCSRFToken(String sessionId, String token) {
        if (sessionId == null || token == null) {
            return false;
        }
        
        String storedToken = sessionTokens.get(sessionId);
        if (storedToken == null) {
            return false;
        }
        
        // Use constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(storedToken.getBytes(), token.getBytes());
    }
    
    public static String updateUserSettings(String sessionId, String csrfToken, 
                                           String username, String email, String theme) {
        // Input validation
        if (username == null || username.trim().isEmpty()) {
            return "Error: Username cannot be empty";
        }
        
        if (email == null || !email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$")) {
            return "Error: Invalid email format";
        }
        
        if (theme == null || theme.trim().isEmpty()) {
            return "Error: Theme cannot be empty";
        }
        
        // CSRF Token validation
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "Error: Invalid CSRF token. Request rejected for security reasons.";
        }
        
        // Sanitize inputs to prevent XSS
        username = sanitizeInput(username);
        email = sanitizeInput(email);
        theme = sanitizeInput(theme);
        
        // Update user settings
        UserSettings settings = new UserSettings(username, email, theme);
        userDatabase.put(sessionId, settings);
        
        // Invalidate the used token and generate a new one
        sessionTokens.remove(sessionId);
        String newToken = generateCSRFToken(sessionId);
        
        return "Success: Settings updated. New CSRF token: " + newToken;
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'&]", "")
                   .trim()
                   .substring(0, Math.min(input.length(), 255));
    }
    
    public static void main(String[] args) {
        System.out.println("=== CSRF-Protected User Settings Update Demo ===\\n");
        
        // Test Case 1: Valid update with correct CSRF token
        System.out.println("Test Case 1: Valid update");
        String session1 = "session_" + UUID.randomUUID().toString();
        String token1 = generateCSRFToken(session1);
        System.out.println("Generated CSRF Token: " + token1);
        String result1 = updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark");
        System.out.println(result1 + "\\n");
        
        // Test Case 2: Invalid CSRF token (should fail)
        System.out.println("Test Case 2: Invalid CSRF token");
        String session2 = "session_" + UUID.randomUUID().toString();
        generateCSRFToken(session2);
        String result2 = updateUserSettings(session2, "invalid_token_12345", "jane_doe", "jane@example.com", "light");
        System.out.println(result2 + "\\n");
        
        // Test Case 3: Reusing token (should fail)
        System.out.println("Test Case 3: Token reuse attempt");
        String session3 = "session_" + UUID.randomUUID().toString();
        String token3 = generateCSRFToken(session3);
        updateUserSettings(session3, token3, "alice", "alice@example.com", "blue");
        String result3 = updateUserSettings(session3, token3, "alice_updated", "alice2@example.com", "red");
        System.out.println(result3 + "\\n");
        
        // Test Case 4: Invalid email format
        System.out.println("Test Case 4: Invalid email format");
        String session4 = "session_" + UUID.randomUUID().toString();
        String token4 = generateCSRFToken(session4);
        String result4 = updateUserSettings(session4, token4, "bob", "invalid-email", "green");
        System.out.println(result4 + "\\n");
        
        // Test Case 5: Empty username
        System.out.println("Test Case 5: Empty username");
        String session5 = "session_" + UUID.randomUUID().toString();
        String token5 = generateCSRFToken(session5);
        String result5 = updateUserSettings(session5, token5, "", "test@example.com", "yellow");
        System.out.println(result5 + "\\n");
    }
}
