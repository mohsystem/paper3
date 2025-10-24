import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task43 {

    // Use ConcurrentHashMap for thread-safe access in a web server environment.
    private final Map<String, SessionData> sessionStore = new ConcurrentHashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder();

    // Inner class to hold session data, e.g., username and creation time.
    private static class SessionData {
        private final String username;
        private final long creationTime;

        public SessionData(String username) {
            this.username = username;
            this.creationTime = System.currentTimeMillis();
        }

        public String getUsername() {
            return username;
        }
        
        // In a real application, you would add an expiration check here.
        // public boolean isExpired(long timeoutMillis) {
        //     return System.currentTimeMillis() - creationTime > timeoutMillis;
        // }
    }

    /**
     * Generates a new, cryptographically strong session ID.
     * @return A secure, URL-safe session ID string.
     */
    private String generateNewSessionId() {
        byte[] randomBytes = new byte[32]; // 256 bits of entropy
        secureRandom.nextBytes(randomBytes);
        return base64Encoder.encodeToString(randomBytes);
    }

    /**
     * Creates a new session for a given user.
     * @param username The username for whom to create the session.
     * @return The newly created session ID.
     */
    public String createSession(String username) {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty.");
        }
        String sessionId = generateNewSessionId();
        sessionStore.put(sessionId, new SessionData(username));
        return sessionId;
    }

    /**
     * Retrieves the username associated with a given session ID.
     * This also serves as session validation.
     * @param sessionId The session ID to look up.
     * @return The username, or null if the session is invalid or not found.
     */
    public String getSessionUser(String sessionId) {
        if (sessionId == null) {
            return null;
        }
        SessionData data = sessionStore.get(sessionId);
        if (data != null) {
            // Optional: Add expiration check here
            // if (data.isExpired(SESSION_TIMEOUT_MS)) {
            //     invalidateSession(sessionId);
            //     return null;
            // }
            return data.getUsername();
        }
        return null;
    }

    /**
     * Terminates/invalidates a session.
     * @param sessionId The session ID to terminate.
     */
    public void invalidateSession(String sessionId) {
        if (sessionId != null) {
            sessionStore.remove(sessionId);
        }
    }

    public static void main(String[] args) {
        Task43 sessionManager = new Task43();
        System.out.println("--- Secure Session Management Test ---");

        // Test Case 1: Create a session for user 'alice'
        System.out.println("\n1. Creating session for 'alice'...");
        String aliceSessionId = sessionManager.createSession("alice");
        System.out.println("   Alice's session ID created: " + aliceSessionId);
        String user = sessionManager.getSessionUser(aliceSessionId);
        System.out.println("   Verified user for session ID: " + user);
        assert "alice".equals(user);

        // Test Case 2: Create a session for user 'bob'
        System.out.println("\n2. Creating session for 'bob'...");
        String bobSessionId = sessionManager.createSession("bob");
        System.out.println("   Bob's session ID created: " + bobSessionId);
        user = sessionManager.getSessionUser(bobSessionId);
        System.out.println("   Verified user for session ID: " + user);
        assert "bob".equals(user);

        // Test Case 3: Validate an invalid/non-existent session ID
        System.out.println("\n3. Validating a non-existent session ID...");
        String fakeSessionId = "invalid-session-id-12345";
        user = sessionManager.getSessionUser(fakeSessionId);
        System.out.println("   User for fake session ID: " + user);
        assert user == null;

        // Test Case 4: Invalidate Alice's session and try to use it again
        System.out.println("\n4. Invalidating Alice's session...");
        sessionManager.invalidateSession(aliceSessionId);
        System.out.println("   Alice's session has been invalidated.");
        user = sessionManager.getSessionUser(aliceSessionId);
        System.out.println("   Attempting to verify Alice's old session ID: " + user);
        assert user == null;

        // Test Case 5: Ensure Bob's session is still valid
        System.out.println("\n5. Verifying Bob's session is still active...");
        user = sessionManager.getSessionUser(bobSessionId);
        System.out.println("   User for Bob's session ID: " + user);
        assert "bob".equals(user);

        System.out.println("\n--- All test cases passed ---");
    }
}