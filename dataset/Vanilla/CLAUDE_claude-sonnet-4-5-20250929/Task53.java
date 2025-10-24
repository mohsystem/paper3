
import java.util.*;
import java.time.LocalDateTime;
import java.time.Duration;

class Task53 {
    private Map<String, Session> sessions;
    private int sessionTimeout; // in minutes
    
    static class Session {
        String sessionId;
        String userId;
        LocalDateTime createdAt;
        LocalDateTime lastAccessedAt;
        Map<String, Object> data;
        
        public Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = LocalDateTime.now();
            this.lastAccessedAt = LocalDateTime.now();
            this.data = new HashMap<>();
        }
        
        public void updateLastAccessed() {
            this.lastAccessedAt = LocalDateTime.now();
        }
        
        public boolean isExpired(int timeoutMinutes) {
            Duration duration = Duration.between(lastAccessedAt, LocalDateTime.now());
            return duration.toMinutes() >= timeoutMinutes;
        }
    }
    
    public Task53(int sessionTimeout) {
        this.sessions = new HashMap<>();
        this.sessionTimeout = sessionTimeout;
    }
    
    public String createSession(String userId) {
        String sessionId = UUID.randomUUID().toString();
        Session session = new Session(sessionId, userId);
        sessions.put(sessionId, session);
        return sessionId;
    }
    
    public boolean validateSession(String sessionId) {
        Session session = sessions.get(sessionId);
        if (session == null) {
            return false;
        }
        if (session.isExpired(sessionTimeout)) {
            sessions.remove(sessionId);
            return false;
        }
        session.updateLastAccessed();
        return true;
    }
    
    public void destroySession(String sessionId) {
        sessions.remove(sessionId);
    }
    
    public void setSessionData(String sessionId, String key, Object value) {
        Session session = sessions.get(sessionId);
        if (session != null && !session.isExpired(sessionTimeout)) {
            session.data.put(key, value);
            session.updateLastAccessed();
        }
    }
    
    public Object getSessionData(String sessionId, String key) {
        Session session = sessions.get(sessionId);
        if (session != null && !session.isExpired(sessionTimeout)) {
            session.updateLastAccessed();
            return session.data.get(key);
        }
        return null;
    }
    
    public String getUserId(String sessionId) {
        Session session = sessions.get(sessionId);
        if (session != null && !session.isExpired(sessionTimeout)) {
            return session.userId;
        }
        return null;
    }
    
    public void cleanupExpiredSessions() {
        List<String> expiredSessions = new ArrayList<>();
        for (Map.Entry<String, Session> entry : sessions.entrySet()) {
            if (entry.getValue().isExpired(sessionTimeout)) {
                expiredSessions.add(entry.getKey());
            }
        }
        for (String sessionId : expiredSessions) {
            sessions.remove(sessionId);
        }
    }
    
    public int getActiveSessionCount() {
        cleanupExpiredSessions();
        return sessions.size();
    }
    
    public static void main(String[] args) throws InterruptedException {
        System.out.println("Test Case 1: Create and validate session");
        Task53 manager1 = new Task53(30);
        String sessionId1 = manager1.createSession("user123");
        System.out.println("Session created: " + sessionId1);
        System.out.println("Session valid: " + manager1.validateSession(sessionId1));
        System.out.println("User ID: " + manager1.getUserId(sessionId1));
        System.out.println();
        
        System.out.println("Test Case 2: Store and retrieve session data");
        Task53 manager2 = new Task53(30);
        String sessionId2 = manager2.createSession("user456");
        manager2.setSessionData(sessionId2, "username", "JohnDoe");
        manager2.setSessionData(sessionId2, "email", "john@example.com");
        System.out.println("Username: " + manager2.getSessionData(sessionId2, "username"));
        System.out.println("Email: " + manager2.getSessionData(sessionId2, "email"));
        System.out.println();
        
        System.out.println("Test Case 3: Destroy session");
        Task53 manager3 = new Task53(30);
        String sessionId3 = manager3.createSession("user789");
        System.out.println("Before destroy - Valid: " + manager3.validateSession(sessionId3));
        manager3.destroySession(sessionId3);
        System.out.println("After destroy - Valid: " + manager3.validateSession(sessionId3));
        System.out.println();
        
        System.out.println("Test Case 4: Multiple sessions");
        Task53 manager4 = new Task53(30);
        String session1 = manager4.createSession("user1");
        String session2 = manager4.createSession("user2");
        String session3 = manager4.createSession("user3");
        System.out.println("Active sessions: " + manager4.getActiveSessionCount());
        manager4.destroySession(session2);
        System.out.println("After destroying one: " + manager4.getActiveSessionCount());
        System.out.println();
        
        System.out.println("Test Case 5: Invalid session operations");
        Task53 manager5 = new Task53(30);
        System.out.println("Validate non-existent session: " + manager5.validateSession("invalid-id"));
        System.out.println("Get data from non-existent session: " + manager5.getSessionData("invalid-id", "key"));
        String validSession = manager5.createSession("user999");
        manager5.destroySession(validSession);
        System.out.println("Get user from destroyed session: " + manager5.getUserId(validSession));
    }
}
