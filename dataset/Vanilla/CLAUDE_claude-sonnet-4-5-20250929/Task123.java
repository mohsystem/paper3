
import java.util.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

class Task123 {
    private Map<String, Session> sessions;
    private DateTimeFormatter formatter;
    
    static class Session {
        String userId;
        String sessionId;
        LocalDateTime loginTime;
        LocalDateTime lastActivityTime;
        boolean active;
        
        Session(String userId, String sessionId, LocalDateTime loginTime) {
            this.userId = userId;
            this.sessionId = sessionId;
            this.loginTime = loginTime;
            this.lastActivityTime = loginTime;
            this.active = true;
        }
    }
    
    public Task123() {
        sessions = new HashMap<>();
        formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    }
    
    public String createSession(String userId) {
        String sessionId = UUID.randomUUID().toString();
        Session session = new Session(userId, sessionId, LocalDateTime.now());
        sessions.put(sessionId, session);
        return sessionId;
    }
    
    public boolean isSessionActive(String sessionId) {
        Session session = sessions.get(sessionId);
        return session != null && session.active;
    }
    
    public void updateActivity(String sessionId) {
        Session session = sessions.get(sessionId);
        if (session != null && session.active) {
            session.lastActivityTime = LocalDateTime.now();
        }
    }
    
    public void endSession(String sessionId) {
        Session session = sessions.get(sessionId);
        if (session != null) {
            session.active = false;
        }
    }
    
    public String getSessionInfo(String sessionId) {
        Session session = sessions.get(sessionId);
        if (session == null) {
            return "Session not found";
        }
        return String.format("User: %s, Active: %s, Login: %s, Last Activity: %s",
            session.userId, session.active,
            session.loginTime.format(formatter),
            session.lastActivityTime.format(formatter));
    }
    
    public List<String> getActiveSessions(String userId) {
        List<String> result = new ArrayList<>();
        for (Session session : sessions.values()) {
            if (session.userId.equals(userId) && session.active) {
                result.add(session.sessionId);
            }
        }
        return result;
    }
    
    public int getActiveSessionCount() {
        int count = 0;
        for (Session session : sessions.values()) {
            if (session.active) {
                count++;
            }
        }
        return count;
    }
    
    public static void main(String[] args) {
        Task123 manager = new Task123();
        
        System.out.println("Test Case 1: Create sessions for users");
        String session1 = manager.createSession("user1");
        String session2 = manager.createSession("user2");
        String session3 = manager.createSession("user1");
        System.out.println("Created sessions: " + session1.substring(0, 8) + "..., " + 
                           session2.substring(0, 8) + "..., " + session3.substring(0, 8) + "...");
        System.out.println("Active session count: " + manager.getActiveSessionCount());
        System.out.println();
        
        System.out.println("Test Case 2: Check session status");
        System.out.println("Session 1 active: " + manager.isSessionActive(session1));
        System.out.println("Invalid session active: " + manager.isSessionActive("invalid-id"));
        System.out.println();
        
        System.out.println("Test Case 3: Update activity and get session info");
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        manager.updateActivity(session1);
        System.out.println(manager.getSessionInfo(session1));
        System.out.println();
        
        System.out.println("Test Case 4: Get active sessions for user");
        List<String> user1Sessions = manager.getActiveSessions("user1");
        System.out.println("User1 has " + user1Sessions.size() + " active sessions");
        System.out.println();
        
        System.out.println("Test Case 5: End session and verify");
        manager.endSession(session1);
        System.out.println("Session 1 active after ending: " + manager.isSessionActive(session1));
        System.out.println("Active session count: " + manager.getActiveSessionCount());
        user1Sessions = manager.getActiveSessions("user1");
        System.out.println("User1 now has " + user1Sessions.size() + " active sessions");
    }
}
