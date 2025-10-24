
import java.security.SecureRandom;
import java.time.LocalDateTime;
import java.time.temporal.ChronoUnit;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task123 {
    private static final int SESSION_TIMEOUT_MINUTES = 30;
    private static final int TOKEN_LENGTH = 32;
    
    private Map<String, Session> sessions;
    private SecureRandom secureRandom;
    
    public Task123() {
        this.sessions = new ConcurrentHashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    static class Session {
        String userId;
        String token;
        LocalDateTime createdAt;
        LocalDateTime lastAccessedAt;
        
        public Session(String userId, String token) {
            this.userId = userId;
            this.token = token;
            this.createdAt = LocalDateTime.now();
            this.lastAccessedAt = LocalDateTime.now();
        }
        
        public boolean isExpired() {
            return ChronoUnit.MINUTES.between(lastAccessedAt, LocalDateTime.now()) > SESSION_TIMEOUT_MINUTES;
        }
    }
    
    public String createSession(String userId) {
        if (userId == null || userId.trim().isEmpty()) {
            return null;
        }
        
        byte[] tokenBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(tokenBytes);
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
        
        Session session = new Session(userId, token);
        sessions.put(token, session);
        
        return token;
    }
    
    public boolean validateSession(String token) {
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        
        Session session = sessions.get(token);
        if (session == null) {
            return false;
        }
        
        if (session.isExpired()) {
            sessions.remove(token);
            return false;
        }
        
        session.lastAccessedAt = LocalDateTime.now();
        return true;
    }
    
    public String getSessionUser(String token) {
        if (!validateSession(token)) {
            return null;
        }
        
        Session session = sessions.get(token);
        return session != null ? session.userId : null;
    }
    
    public boolean destroySession(String token) {
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        
        return sessions.remove(token) != null;
    }
    
    public void cleanupExpiredSessions() {
        sessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
    }
    
    public static void main(String[] args) {
        Task123 sessionManager = new Task123();
        
        // Test Case 1: Create a session for a user
        System.out.println("Test Case 1: Create Session");
        String token1 = sessionManager.createSession("user123");
        System.out.println("Token created: " + (token1 != null ? "Yes" : "No"));
        System.out.println();
        
        // Test Case 2: Validate an existing session
        System.out.println("Test Case 2: Validate Session");
        boolean isValid = sessionManager.validateSession(token1);
        System.out.println("Session valid: " + isValid);
        System.out.println();
        
        // Test Case 3: Get user from session
        System.out.println("Test Case 3: Get Session User");
        String userId = sessionManager.getSessionUser(token1);
        System.out.println("User ID: " + userId);
        System.out.println();
        
        // Test Case 4: Destroy a session
        System.out.println("Test Case 4: Destroy Session");
        boolean destroyed = sessionManager.destroySession(token1);
        System.out.println("Session destroyed: " + destroyed);
        boolean validAfterDestroy = sessionManager.validateSession(token1);
        System.out.println("Session valid after destroy: " + validAfterDestroy);
        System.out.println();
        
        // Test Case 5: Invalid session token
        System.out.println("Test Case 5: Invalid Session Token");
        boolean invalidToken = sessionManager.validateSession("invalid_token_xyz");
        System.out.println("Invalid token validation: " + invalidToken);
        String userFromInvalid = sessionManager.getSessionUser("invalid_token_xyz");
        System.out.println("User from invalid token: " + userFromInvalid);
    }
}
