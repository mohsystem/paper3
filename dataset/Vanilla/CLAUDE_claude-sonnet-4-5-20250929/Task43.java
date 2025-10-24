
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.time.Instant;
import java.time.temporal.ChronoUnit;

class Task43 {
    private static final int SESSION_TIMEOUT_MINUTES = 30;
    private static final int TOKEN_LENGTH = 32;
    
    static class Session {
        private String sessionId;
        private String userId;
        private Instant createdAt;
        private Instant lastAccessedAt;
        private Map<String, Object> attributes;
        
        public Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = Instant.now();
            this.lastAccessedAt = Instant.now();
            this.attributes = new ConcurrentHashMap<>();
        }
        
        public String getSessionId() { return sessionId; }
        public String getUserId() { return userId; }
        public Instant getCreatedAt() { return createdAt; }
        public Instant getLastAccessedAt() { return lastAccessedAt; }
        
        public void updateLastAccessed() {
            this.lastAccessedAt = Instant.now();
        }
        
        public boolean isExpired() {
            return Instant.now().isAfter(lastAccessedAt.plus(SESSION_TIMEOUT_MINUTES, ChronoUnit.MINUTES));
        }
        
        public void setAttribute(String key, Object value) {
            attributes.put(key, value);
        }
        
        public Object getAttribute(String key) {
            return attributes.get(key);
        }
        
        @Override
        public String toString() {
            return String.format("Session{id='%s', userId='%s', created=%s, lastAccessed=%s, expired=%b}",
                sessionId, userId, createdAt, lastAccessedAt, isExpired());
        }
    }
    
    static class SessionManager {
        private Map<String, Session> sessions;
        private SecureRandom secureRandom;
        
        public SessionManager() {
            this.sessions = new ConcurrentHashMap<>();
            this.secureRandom = new SecureRandom();
        }
        
        public String createSession(String userId) {
            String sessionId = generateSessionId();
            Session session = new Session(sessionId, userId);
            sessions.put(sessionId, session);
            cleanupExpiredSessions();
            return sessionId;
        }
        
        public Session getSession(String sessionId) {
            Session session = sessions.get(sessionId);
            if (session != null) {
                if (session.isExpired()) {
                    terminateSession(sessionId);
                    return null;
                }
                session.updateLastAccessed();
            }
            return session;
        }
        
        public boolean validateSession(String sessionId) {
            return getSession(sessionId) != null;
        }
        
        public void terminateSession(String sessionId) {
            sessions.remove(sessionId);
        }
        
        public void terminateUserSessions(String userId) {
            sessions.entrySet().removeIf(entry -> entry.getValue().getUserId().equals(userId));
        }
        
        public int getActiveSessionCount() {
            cleanupExpiredSessions();
            return sessions.size();
        }
        
        private String generateSessionId() {
            byte[] randomBytes = new byte[TOKEN_LENGTH];
            secureRandom.nextBytes(randomBytes);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
        }
        
        private void cleanupExpiredSessions() {
            sessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
        }
    }
    
    public static void main(String[] args) {
        SessionManager manager = new SessionManager();
        
        System.out.println("=== Test Case 1: Create Session ===");
        String sessionId1 = manager.createSession("user123");
        System.out.println("Created session: " + sessionId1);
        System.out.println("Session valid: " + manager.validateSession(sessionId1));
        
        System.out.println("\\n=== Test Case 2: Get Session and Update Access Time ===");
        Session session1 = manager.getSession(sessionId1);
        System.out.println(session1);
        session1.setAttribute("username", "JohnDoe");
        System.out.println("Username attribute: " + session1.getAttribute("username"));
        
        System.out.println("\\n=== Test Case 3: Multiple Sessions ===");
        String sessionId2 = manager.createSession("user456");
        String sessionId3 = manager.createSession("user789");
        System.out.println("Active sessions: " + manager.getActiveSessionCount());
        
        System.out.println("\\n=== Test Case 4: Terminate Specific Session ===");
        manager.terminateSession(sessionId2);
        System.out.println("Session 2 valid after termination: " + manager.validateSession(sessionId2));
        System.out.println("Active sessions: " + manager.getActiveSessionCount());
        
        System.out.println("\\n=== Test Case 5: Terminate All User Sessions ===");
        String sessionId4 = manager.createSession("user123");
        System.out.println("Created another session for user123");
        System.out.println("Active sessions before cleanup: " + manager.getActiveSessionCount());
        manager.terminateUserSessions("user123");
        System.out.println("Active sessions after user123 cleanup: " + manager.getActiveSessionCount());
        System.out.println("Session 1 valid: " + manager.validateSession(sessionId1));
        System.out.println("Session 4 valid: " + manager.validateSession(sessionId4));
    }
}
