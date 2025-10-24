
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.spec.KeySpec;
import java.util.Base64;

class Task43 {
    private static final int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    private static final int SESSION_ID_LENGTH = 32;
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();
    
    static class Session {
        String sessionId;
        String userId;
        Instant createdAt;
        Instant lastAccessedAt;
        Map<String, Object> attributes;
        
        Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = Instant.now();
            this.lastAccessedAt = Instant.now();
            this.attributes = new HashMap<>();
        }
        
        boolean isExpired() {
            return Instant.now().getEpochSecond() - lastAccessedAt.getEpochSecond() > SESSION_TIMEOUT_SECONDS;
        }
        
        void updateLastAccessed() {
            this.lastAccessedAt = Instant.now();
        }
    }
    
    public static String createSession(String userId) {
        if (userId == null || userId.trim().isEmpty()) {
            return null;
        }
        
        cleanExpiredSessions();
        
        byte[] randomBytes = new byte[SESSION_ID_LENGTH];
        secureRandom.nextBytes(randomBytes);
        String sessionId = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
        
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
        return sessions.get(sessionId).userId;
    }
    
    public static boolean terminateSession(String sessionId) {
        if (sessionId == null) {
            return false;
        }
        return sessions.remove(sessionId) != null;
    }
    
    public static void cleanExpiredSessions() {
        sessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
    }
    
    public static boolean setSessionAttribute(String sessionId, String key, Object value) {
        if (!validateSession(sessionId) || key == null) {
            return false;
        }
        sessions.get(sessionId).attributes.put(key, value);
        return true;
    }
    
    public static Object getSessionAttribute(String sessionId, String key) {
        if (!validateSession(sessionId) || key == null) {
            return null;
        }
        return sessions.get(sessionId).attributes.get(key);
    }
    
    public static int getActiveSessionCount() {
        cleanExpiredSessions();
        return sessions.size();
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Session Management Test Cases ===\\n");
        
        // Test Case 1: Create and validate session
        System.out.println("Test Case 1: Create and Validate Session");
        String session1 = createSession("user123");
        System.out.println("Session created: " + (session1 != null));
        System.out.println("Session valid: " + validateSession(session1));
        System.out.println("User ID: " + getSessionUserId(session1));
        System.out.println();
        
        // Test Case 2: Session attributes
        System.out.println("Test Case 2: Session Attributes");
        String session2 = createSession("user456");
        setSessionAttribute(session2, "role", "admin");
        setSessionAttribute(session2, "loginTime", Instant.now().toString());
        System.out.println("Role attribute: " + getSessionAttribute(session2, "role"));
        System.out.println("Login time set: " + (getSessionAttribute(session2, "loginTime") != null));
        System.out.println();
        
        // Test Case 3: Invalid session handling
        System.out.println("Test Case 3: Invalid Session Handling");
        System.out.println("Null session valid: " + validateSession(null));
        System.out.println("Empty session valid: " + validateSession(""));
        System.out.println("Fake session valid: " + validateSession("fake-session-id"));
        System.out.println();
        
        // Test Case 4: Session termination
        System.out.println("Test Case 4: Session Termination");
        String session3 = createSession("user789");
        System.out.println("Session created: " + validateSession(session3));
        boolean terminated = terminateSession(session3);
        System.out.println("Session terminated: " + terminated);
        System.out.println("Session still valid: " + validateSession(session3));
        System.out.println();
        
        // Test Case 5: Active session count
        System.out.println("Test Case 5: Active Session Count");
        createSession("user001");
        createSession("user002");
        createSession("user003");
        int count = getActiveSessionCount();
        System.out.println("Active sessions: " + count);
        cleanExpiredSessions();
        System.out.println("After cleanup: " + getActiveSessionCount());
        System.out.println();
    }
}
