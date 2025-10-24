import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

class Session {
    private final String username;
    private final long expiryTime;

    public Session(String username, long expiryTime) {
        this.username = username;
        this.expiryTime = expiryTime;
    }

    public String getUsername() {
        return username;
    }

    public boolean isExpired() {
        return System.currentTimeMillis() > expiryTime;
    }
}

class SessionManager {
    // Use a ConcurrentHashMap for thread-safe session storage
    private final ConcurrentHashMap<String, Session> sessions;
    private final long sessionTimeoutMillis;

    public SessionManager(long sessionTimeout, TimeUnit unit) {
        this.sessions = new ConcurrentHashMap<>();
        this.sessionTimeoutMillis = unit.toMillis(sessionTimeout);
    }

    /**
     * Creates a new session for a user.
     * @param username The username for whom to create the session.
     * @return A unique session ID.
     */
    public String createSession(String username) {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty.");
        }
        // UUID.randomUUID() generates a cryptographically strong random ID
        String sessionId = UUID.randomUUID().toString();
        long expiryTime = System.currentTimeMillis() + sessionTimeoutMillis;
        Session session = new Session(username, expiryTime);
        sessions.put(sessionId, session);
        return sessionId;
    }

    /**
     * Checks if a session ID is valid and not expired.
     * @param sessionId The session ID to validate.
     * @return true if the session is valid, false otherwise.
     */
    public boolean isValidSession(String sessionId) {
        if (sessionId == null) {
            return false;
        }
        Session session = sessions.get(sessionId);
        if (session == null) {
            return false;
        }
        if (session.isExpired()) {
            // Lazy cleanup of expired sessions
            sessions.remove(sessionId, session);
            return false;
        }
        return true;
    }
    
    /**
     * Retrieves the username associated with a valid session.
     * @param sessionId The session ID.
     * @return The username, or null if the session is invalid.
     */
    public String getUsername(String sessionId) {
        if (!isValidSession(sessionId)) {
            return null;
        }
        return sessions.get(sessionId).getUsername();
    }

    /**
     * Invalidates/removes a session.
     * @param sessionId The session ID to invalidate.
     */
    public void invalidateSession(String sessionId) {
        if (sessionId != null) {
            sessions.remove(sessionId);
        }
    }
}


public class Task123 {
    public static void main(String[] args) {
        System.out.println("--- Java Session Manager ---");
        // Test Case 1: Create and validate a new session
        System.out.println("\n--- Test Case 1: Create and Validate Session ---");
        SessionManager manager = new SessionManager(5, TimeUnit.MINUTES);
        String sessionId1 = manager.createSession("userA");
        System.out.println("Created session for userA: " + sessionId1);
        System.out.println("Is session valid? " + manager.isValidSession(sessionId1));
        System.out.println("Username from session: " + manager.getUsername(sessionId1));

        // Test Case 2: Invalidate a session
        System.out.println("\n--- Test Case 2: Invalidate Session ---");
        manager.invalidateSession(sessionId1);
        System.out.println("Session for userA invalidated.");
        System.out.println("Is session valid after invalidation? " + manager.isValidSession(sessionId1));

        // Test Case 3: Validate a non-existent session
        System.out.println("\n--- Test Case 3: Validate Non-existent Session ---");
        String fakeSessionId = UUID.randomUUID().toString();
        System.out.println("Is a fake session ID valid? " + manager.isValidSession(fakeSessionId));

        // Test Case 4: Session expiration
        System.out.println("\n--- Test Case 4: Session Expiration ---");
        SessionManager shortLivedManager = new SessionManager(2, TimeUnit.SECONDS);
        String sessionId2 = shortLivedManager.createSession("userB");
        System.out.println("Created short-lived session for userB: " + sessionId2);
        System.out.println("Is session valid immediately? " + shortLivedManager.isValidSession(sessionId2));
        System.out.println("Waiting for 3 seconds for session to expire...");
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("Is session valid after expiration? " + shortLivedManager.isValidSession(sessionId2));

        // Test Case 5: Attempt to create session with invalid username
        System.out.println("\n--- Test Case 5: Invalid Username ---");
        try {
            manager.createSession(null);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for null username: " + e.getMessage());
        }
        try {
            manager.createSession("   ");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for empty username: " + e.getMessage());
        }
    }
}