import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task53 {

    // Use ConcurrentHashMap for thread-safe session management.
    private final Map<String, Session> activeSessions = new ConcurrentHashMap<>();
    
    // Set a short timeout for demonstration purposes (e.g., 3 seconds).
    private static final long SESSION_TIMEOUT_SECONDS = 3;

    /**
     * Inner class to store session data.
     */
    private static class Session {
        private final String username;
        private final long creationTimeMillis;

        public Session(String username, long creationTimeMillis) {
            this.username = username;
            this.creationTimeMillis = creationTimeMillis;
        }

        public String getUsername() {
            return username;
        }

        public long getCreationTimeMillis() {
            return creationTimeMillis;
        }
    }

    /**
     * Creates a new session for a user.
     * Generates a cryptographically secure, unique session ID.
     * @param username The username for whom to create the session.
     * @return The generated session ID.
     */
    public String createSession(String username) {
        if (username == null || username.trim().isEmpty()) {
            return null; // Do not create sessions for invalid usernames
        }

        // Use SecureRandom for cryptographically strong random number generation.
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[32]; // 256 bits of entropy
        random.nextBytes(bytes);
        
        // Use URL-safe Base64 encoding for the session ID.
        String sessionId = Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);

        Session session = new Session(username, System.currentTimeMillis());
        activeSessions.put(sessionId, session);
        System.out.println("Session created for " + username + " with ID: " + sessionId);
        return sessionId;
    }

    /**
     * Checks if a session is valid and not expired.
     * @param sessionId The session ID to validate.
     * @return true if the session is valid, false otherwise.
     */
    public boolean isSessionValid(String sessionId) {
        if (sessionId == null) {
            return false;
        }

        Session session = activeSessions.get(sessionId);
        if (session == null) {
            return false; // Session does not exist.
        }

        long currentTime = System.currentTimeMillis();
        long sessionAgeSeconds = (currentTime - session.getCreationTimeMillis()) / 1000;

        if (sessionAgeSeconds > SESSION_TIMEOUT_SECONDS) {
            // Session has expired, remove it.
            activeSessions.remove(sessionId);
            System.out.println("Session " + sessionId + " expired.");
            return false;
        }
        
        // In a real application, you might update a "last accessed" timestamp here.
        return true;
    }

    /**
     * Invalidates/removes a user session.
     * @param sessionId The session ID to invalidate.
     */
    public void invalidateSession(String sessionId) {
        if (sessionId != null) {
            if(activeSessions.remove(sessionId) != null) {
                System.out.println("Session " + sessionId + " invalidated.");
            } else {
                 System.out.println("Session " + sessionId + " not found for invalidation.");
            }
        }
    }

    public static void main(String[] args) throws InterruptedException {
        Task53 sessionManager = new Task53();
        System.out.println("--- Session Management Test ---");

        // Test Case 1: Create a session and validate it
        System.out.println("\n--- Test Case 1: Create and Validate ---");
        String user1SessionId = sessionManager.createSession("user1");
        System.out.println("Is user1's session valid? " + sessionManager.isSessionValid(user1SessionId));

        // Test Case 2: Validate a non-existent session
        System.out.println("\n--- Test Case 2: Validate Non-Existent Session ---");
        System.out.println("Is 'invalid-session-id' valid? " + sessionManager.isSessionValid("invalid-session-id"));

        // Test Case 3: Invalidate a session and check it again
        System.out.println("\n--- Test Case 3: Invalidate and Re-Validate ---");
        sessionManager.invalidateSession(user1SessionId);
        System.out.println("Is user1's session valid after invalidation? " + sessionManager.isSessionValid(user1SessionId));

        // Test Case 4: Test session timeout
        System.out.println("\n--- Test Case 4: Session Timeout ---");
        String user2SessionId = sessionManager.createSession("user2");
        System.out.println("Is user2's session valid immediately? " + sessionManager.isSessionValid(user2SessionId));
        System.out.println("Waiting for " + (SESSION_TIMEOUT_SECONDS + 1) + " seconds to test timeout...");
        TimeUnit.SECONDS.sleep(SESSION_TIMEOUT_SECONDS + 1);
        System.out.println("Is user2's session valid after timeout? " + sessionManager.isSessionValid(user2SessionId));

        // Test Case 5: Create multiple sessions
        System.out.println("\n--- Test Case 5: Multiple Sessions ---");
        String user3SessionId = sessionManager.createSession("user3");
        String user4SessionId = sessionManager.createSession("user4");
        System.out.println("Is user3's session valid? " + sessionManager.isSessionValid(user3SessionId));
        System.out.println("Is user4's session valid? " + sessionManager.isSessionValid(user4SessionId));
        sessionManager.invalidateSession(user3SessionId);
        System.out.println("Is user3's session valid after invalidation? " + sessionManager.isSessionValid(user3SessionId));
        System.out.println("Is user4's session still valid? " + sessionManager.isSessionValid(user4SessionId));
    }
}