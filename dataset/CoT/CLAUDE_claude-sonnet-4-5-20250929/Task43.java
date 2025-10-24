
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.time.Instant;
import java.time.Duration;

class Task43 {
    private static final int SESSION_TIMEOUT_MINUTES = 30;
    private static final int SESSION_ID_LENGTH = 32;
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();
    
    static class Session {
        private final String sessionId;
        private final String userId;
        private final Instant createdAt;
        private Instant lastAccessedAt;
        private final Map<String, String> attributes;
        
        public Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = Instant.now();
            this.lastAccessedAt = Instant.now();
            this.attributes = new ConcurrentHashMap<>();
        }
        
        public boolean isExpired() {
            Duration duration = Duration.between(lastAccessedAt, Instant.now());
            return duration.toMinutes() >= SESSION_TIMEOUT_MINUTES;
        }
        
        public void updateLastAccessed() {
            this.lastAccessedAt = Instant.now();
        }
        
        public String getSessionId() { return sessionId; }
        public String getUserId() { return userId; }
        public void setAttribute(String key, String value) {
            if (key != null && value != null) {
                attributes.put(key, value);
            }
        }
        public String getAttribute(String key) { return attributes.get(key); }
    }
    
    public static String createSession(String userId) {
        if (userId == null || userId.trim().isEmpty()) {
            throw new IllegalArgumentException("User ID cannot be null or empty");
        }
        
        cleanExpiredSessions();
        
        byte[] randomBytes = new byte[SESSION_ID_LENGTH];
        secureRandom.nextBytes(randomBytes);
        
        String sessionId = Base64.getUrlEncoder().withoutPadding()
            .encodeToString(hashBytes(randomBytes));
        
        Session session = new Session(sessionId, userId);
        sessions.put(sessionId, session);
        
        return sessionId;
    }
    
    public static boolean validateSession(String sessionId) {
        if (sessionId == null || sessionId.trim().isEmpty()) {
            return false;
        }
        
        Session session = sessions.get(sessionId);
        if (session == null) {
            return false;
        }
        
        if (session.isExpired()) {
            terminateSession(sessionId);
            return false;
        }
        
        session.updateLastAccessed();
        return true;
    }
    
    public static String getSessionUserId(String sessionId) {
        if (!validateSession(sessionId)) {
            return null;
        }
        
        Session session = sessions.get(sessionId);
        return session != null ? session.getUserId() : null;
    }
    
    public static void terminateSession(String sessionId) {
        if (sessionId != null) {
            Session session = sessions.remove(sessionId);
            if (session != null) {
                session.attributes.clear();
            }
        }
    }
    
    public static void setSessionAttribute(String sessionId, String key, String value) {
        if (validateSession(sessionId)) {
            Session session = sessions.get(sessionId);
            if (session != null && key != null && value != null) {
                session.setAttribute(key, value);
            }
        }
    }
    
    public static String getSessionAttribute(String sessionId, String key) {
        if (validateSession(sessionId)) {
            Session session = sessions.get(sessionId);
            return session != null ? session.getAttribute(key) : null;
        }
        return null;
    }
    
    private static void cleanExpiredSessions() {
        sessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
    }
    
    private static byte[] hashBytes(byte[] input) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            return digest.digest(input);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Session Management Test Cases ===\\n");
        
        // Test Case 1: Create session for user
        System.out.println("Test 1: Create session");
        String sessionId1 = createSession("user123");
        System.out.println("Session created: " + sessionId1);
        System.out.println("Valid: " + validateSession(sessionId1) + "\\n");
        
        // Test Case 2: Validate existing session
        System.out.println("Test 2: Validate session");
        boolean isValid = validateSession(sessionId1);
        System.out.println("Session valid: " + isValid);
        System.out.println("User ID: " + getSessionUserId(sessionId1) + "\\n");
        
        // Test Case 3: Set and get session attributes
        System.out.println("Test 3: Session attributes");
        setSessionAttribute(sessionId1, "role", "admin");
        setSessionAttribute(sessionId1, "preference", "dark_mode");
        System.out.println("Role: " + getSessionAttribute(sessionId1, "role"));
        System.out.println("Preference: " + getSessionAttribute(sessionId1, "preference") + "\\n");
        
        // Test Case 4: Multiple sessions
        System.out.println("Test 4: Multiple sessions");
        String sessionId2 = createSession("user456");
        String sessionId3 = createSession("user789");
        System.out.println("Session 2 created: " + sessionId2);
        System.out.println("Session 3 created: " + sessionId3);
        System.out.println("Session 1 valid: " + validateSession(sessionId1));
        System.out.println("Session 2 valid: " + validateSession(sessionId2));
        System.out.println("Session 3 valid: " + validateSession(sessionId3) + "\\n");
        
        // Test Case 5: Terminate session
        System.out.println("Test 5: Terminate session");
        terminateSession(sessionId1);
        System.out.println("Session 1 after termination: " + validateSession(sessionId1));
        System.out.println("Session 2 still valid: " + validateSession(sessionId2));
        System.out.println("Session 3 still valid: " + validateSession(sessionId3));
        
        // Cleanup
        terminateSession(sessionId2);
        terminateSession(sessionId3);
    }
}
