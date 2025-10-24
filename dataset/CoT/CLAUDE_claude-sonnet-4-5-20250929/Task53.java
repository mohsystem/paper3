
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task53 {
    private static final int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    private static final int ABSOLUTE_TIMEOUT_SECONDS = 7200; // 2 hours
    private static final int SESSION_ID_LENGTH = 32;
    
    private Map<String, Session> sessions;
    private Map<String, Set<String>> userSessions;
    private SecureRandom secureRandom;
    
    public Task53() {
        this.sessions = new ConcurrentHashMap<>();
        this.userSessions = new ConcurrentHashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    private static class Session {
        String sessionId;
        String userId;
        long createdAt;
        long lastAccessedAt;
        Map<String, String> attributes;
        
        Session(String sessionId, String userId) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = Instant.now().getEpochSecond();
            this.lastAccessedAt = this.createdAt;
            this.attributes = new HashMap<>();
        }
        
        boolean isExpired() {
            long now = Instant.now().getEpochSecond();
            boolean idleExpired = (now - lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
            boolean absoluteExpired = (now - createdAt) > ABSOLUTE_TIMEOUT_SECONDS;
            return idleExpired || absoluteExpired;
        }
        
        void updateAccess() {
            this.lastAccessedAt = Instant.now().getEpochSecond();
        }
    }
    
    private String generateSessionId() {
        byte[] randomBytes = new byte[SESSION_ID_LENGTH];
        secureRandom.nextBytes(randomBytes);
        StringBuilder sb = new StringBuilder();
        for (byte b : randomBytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
    
    public String createSession(String userId) {
        if (userId == null || userId.trim().isEmpty()) {
            return null;
        }
        
        String sessionId = generateSessionId();
        Session session = new Session(sessionId, userId);
        sessions.put(sessionId, session);
        
        userSessions.computeIfAbsent(userId, k -> ConcurrentHashMap.newKeySet()).add(sessionId);
        
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
        
        if (session.isExpired()) {
            invalidateSession(sessionId);
            return false;
        }
        
        session.updateAccess();
        return true;
    }
    
    public void invalidateSession(String sessionId) {
        Session session = sessions.remove(sessionId);
        if (session != null) {
            Set<String> userSessionSet = userSessions.get(session.userId);
            if (userSessionSet != null) {
                userSessionSet.remove(sessionId);
                if (userSessionSet.isEmpty()) {
                    userSessions.remove(session.userId);
                }
            }
        }
    }
    
    public void invalidateAllUserSessions(String userId) {
        Set<String> userSessionSet = userSessions.get(userId);
        if (userSessionSet != null) {
            for (String sessionId : new HashSet<>(userSessionSet)) {
                sessions.remove(sessionId);
            }
            userSessions.remove(userId);
        }
    }
    
    public int getActiveSessionCount(String userId) {
        Set<String> userSessionSet = userSessions.get(userId);
        if (userSessionSet == null) {
            return 0;
        }
        
        Set<String> validSessions = new HashSet<>();
        for (String sessionId : userSessionSet) {
            if (validateSession(sessionId)) {
                validSessions.add(sessionId);
            }
        }
        return validSessions.size();
    }
    
    public void cleanupExpiredSessions() {
        for (String sessionId : new HashSet<>(sessions.keySet())) {
            Session session = sessions.get(sessionId);
            if (session != null && session.isExpired()) {
                invalidateSession(sessionId);
            }
        }
    }
    
    public static void main(String[] args) throws InterruptedException {
        Task53 sessionManager = new Task53();
        
        // Test Case 1: Create and validate session
        System.out.println("Test Case 1: Create and validate session");
        String session1 = sessionManager.createSession("user1");
        System.out.println("Session created: " + (session1 != null));
        System.out.println("Session valid: " + sessionManager.validateSession(session1));
        System.out.println();
        
        // Test Case 2: Multiple sessions for same user
        System.out.println("Test Case 2: Multiple sessions for same user");
        String session2 = sessionManager.createSession("user1");
        String session3 = sessionManager.createSession("user1");
        System.out.println("Active sessions for user1: " + sessionManager.getActiveSessionCount("user1"));
        System.out.println();
        
        // Test Case 3: Invalidate specific session
        System.out.println("Test Case 3: Invalidate specific session");
        sessionManager.invalidateSession(session2);
        System.out.println("Session2 valid after invalidation: " + sessionManager.validateSession(session2));
        System.out.println("Active sessions for user1: " + sessionManager.getActiveSessionCount("user1"));
        System.out.println();
        
        // Test Case 4: Invalidate all user sessions
        System.out.println("Test Case 4: Invalidate all user sessions");
        sessionManager.invalidateAllUserSessions("user1");
        System.out.println("Session1 valid after user invalidation: " + sessionManager.validateSession(session1));
        System.out.println("Active sessions for user1: " + sessionManager.getActiveSessionCount("user1"));
        System.out.println();
        
        // Test Case 5: Invalid session handling
        System.out.println("Test Case 5: Invalid session handling");
        System.out.println("Null session valid: " + sessionManager.validateSession(null));
        System.out.println("Non-existent session valid: " + sessionManager.validateSession("invalid123"));
        String nullUserSession = sessionManager.createSession(null);
        System.out.println("Session created with null user: " + (nullUserSession != null));
    }
}
