
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task53 {
    private static final int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    private static final int TOKEN_LENGTH = 32;
    private static final SecureRandom secureRandom = new SecureRandom();
    
    private final ConcurrentHashMap<String, Session> sessions;
    
    public Task53() {
        this.sessions = new ConcurrentHashMap<>();
    }
    
    static class Session {
        private final String sessionId;
        private final String userId;
        private final long createdAt;
        private long lastAccessedAt;
        private final Map<String, Object> attributes;
        
        public Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = Instant.now().getEpochSecond();
            this.lastAccessedAt = this.createdAt;
            this.attributes = new HashMap<>();
        }
        
        public boolean isExpired(int timeoutSeconds) {
            long currentTime = Instant.now().getEpochSecond();
            return (currentTime - lastAccessedAt) > timeoutSeconds;
        }
        
        public void updateLastAccessed() {
            this.lastAccessedAt = Instant.now().getEpochSecond();
        }
        
        public String getSessionId() { return sessionId; }
        public String getUserId() { return userId; }
        public Map<String, Object> getAttributes() { return attributes; }
    }
    
    public String createSession(String userId) {
        if (userId == null || userId.trim().isEmpty()) {
            return null;
        }
        
        String sessionId = generateSecureToken();
        Session session = new Session(sessionId, userId);
        sessions.put(sessionId, session);
        return sessionId;
    }
    
    public boolean validateSession(String sessionId) {
        if (sessionId == null) {
            return false;
        }
        
        Session session = sessions.get(sessionId);
        if (session == null) {
            return false;
        }
        
        if (session.isExpired(SESSION_TIMEOUT_SECONDS)) {
            sessions.remove(sessionId);
            return false;
        }
        
        session.updateLastAccessed();
        return true;
    }
    
    public String getUserId(String sessionId) {
        if (!validateSession(sessionId)) {
            return null;
        }
        
        Session session = sessions.get(sessionId);
        return session != null ? session.getUserId() : null;
    }
    
    public boolean invalidateSession(String sessionId) {
        if (sessionId == null) {
            return false;
        }
        
        return sessions.remove(sessionId) != null;
    }
    
    public void setAttribute(String sessionId, String key, Object value) {
        if (validateSession(sessionId) && key != null) {
            Session session = sessions.get(sessionId);
            if (session != null) {
                session.getAttributes().put(key, value);
            }
        }
    }
    
    public Object getAttribute(String sessionId, String key) {
        if (validateSession(sessionId) && key != null) {
            Session session = sessions.get(sessionId);
            if (session != null) {
                return session.getAttributes().get(key);
            }
        }
        return null;
    }
    
    public void cleanupExpiredSessions() {
        List<String> expiredSessions = new ArrayList<>();
        
        for (Map.Entry<String, Session> entry : sessions.entrySet()) {
            if (entry.getValue().isExpired(SESSION_TIMEOUT_SECONDS)) {
                expiredSessions.add(entry.getKey());
            }
        }
        
        for (String sessionId : expiredSessions) {
            sessions.remove(sessionId);
        }
    }
    
    private String generateSecureToken() {
        byte[] randomBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(randomBytes);
        
        StringBuilder token = new StringBuilder();
        for (byte b : randomBytes) {
            token.append(String.format("%02x", b));
        }
        
        return token.toString();
    }
    
    public static void main(String[] args) throws InterruptedException {
        Task53 sessionManager = new Task53();
        
        // Test Case 1: Create a session
        System.out.println("Test Case 1: Create Session");
        String session1 = sessionManager.createSession("user123");
        System.out.println("Session created: " + (session1 != null));
        System.out.println("Session ID length: " + session1.length());
        
        // Test Case 2: Validate session
        System.out.println("\\nTest Case 2: Validate Session");
        boolean isValid = sessionManager.validateSession(session1);
        System.out.println("Session valid: " + isValid);
        String userId = sessionManager.getUserId(session1);
        System.out.println("User ID: " + userId);
        
        // Test Case 3: Set and get attributes
        System.out.println("\\nTest Case 3: Session Attributes");
        sessionManager.setAttribute(session1, "username", "JohnDoe");
        sessionManager.setAttribute(session1, "role", "admin");
        Object username = sessionManager.getAttribute(session1, "username");
        Object role = sessionManager.getAttribute(session1, "role");
        System.out.println("Username: " + username);
        System.out.println("Role: " + role);
        
        // Test Case 4: Invalidate session
        System.out.println("\\nTest Case 4: Invalidate Session");
        boolean invalidated = sessionManager.invalidateSession(session1);
        System.out.println("Session invalidated: " + invalidated);
        boolean stillValid = sessionManager.validateSession(session1);
        System.out.println("Session still valid: " + stillValid);
        
        // Test Case 5: Multiple sessions and cleanup
        System.out.println("\\nTest Case 5: Multiple Sessions");
        String session2 = sessionManager.createSession("user456");
        String session3 = sessionManager.createSession("user789");
        System.out.println("Session 2 created: " + (session2 != null));
        System.out.println("Session 3 created: " + (session3 != null));
        sessionManager.cleanupExpiredSessions();
        System.out.println("Cleanup completed");
        System.out.println("Session 2 valid: " + sessionManager.validateSession(session2));
    }
}
