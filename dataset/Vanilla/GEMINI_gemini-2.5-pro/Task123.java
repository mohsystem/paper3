import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Task123 {

    // In-memory store for sessions. Key: Session ID, Value: Session object
    private static final Map<String, Session> sessionStore = new HashMap<>();

    // Inner class to represent a session
    private static class Session {
        String username;
        long creationTime;

        Session(String username) {
            this.username = username;
            this.creationTime = System.currentTimeMillis();
        }

        public String getUsername() {
            return username;
        }
    }

    /**
     * Creates a new session for a given user.
     * @param username The username to associate with the session.
     * @return The unique session ID.
     */
    public static String createSession(String username) {
        String sessionId = UUID.randomUUID().toString();
        Session session = new Session(username);
        sessionStore.put(sessionId, session);
        System.out.println("Session created for " + username + " with ID: " + sessionId);
        return sessionId;
    }

    /**
     * Retrieves the username associated with a session ID.
     * @param sessionId The session ID to look up.
     * @return The username, or null if the session is not valid.
     */
    public static String getSessionUser(String sessionId) {
        if (sessionStore.containsKey(sessionId)) {
            return sessionStore.get(sessionId).getUsername();
        }
        return null;
    }

    /**
     * Invalidates/removes a user session.
     * @param sessionId The session ID to invalidate.
     */
    public static void invalidateSession(String sessionId) {
        if (sessionStore.remove(sessionId) != null) {
            System.out.println("Session invalidated: " + sessionId);
        } else {
            System.out.println("Session to invalidate not found: " + sessionId);
        }
    }

    /**
     * Checks if a session ID is currently valid.
     * @param sessionId The session ID to check.
     * @return true if the session exists, false otherwise.
     */
    public static boolean isSessionValid(String sessionId) {
        return sessionStore.containsKey(sessionId);
    }

    public static void main(String[] args) {
        System.out.println("--- Starting Session Management Test ---");

        // Test Case 1: Create a session for user "alice"
        System.out.println("\n[Test Case 1: Create session for alice]");
        String aliceSessionId = createSession("alice");

        // Test Case 2: Check if alice's session is valid and get her username
        System.out.println("\n[Test Case 2: Validate alice's session]");
        System.out.println("Is alice's session valid? " + isSessionValid(aliceSessionId));
        String username = getSessionUser(aliceSessionId);
        System.out.println("Retrieved user for session " + aliceSessionId + ": " + username);

        // Test Case 3: Create a second session for user "bob"
        System.out.println("\n[Test Case 3: Create session for bob]");
        String bobSessionId = createSession("bob");
        System.out.println("Is bob's session valid? " + isSessionValid(bobSessionId));

        // Test Case 4: Invalidate alice's session and verify
        System.out.println("\n[Test Case 4: Invalidate alice's session]");
        invalidateSession(aliceSessionId);
        System.out.println("Is alice's session now valid? " + isSessionValid(aliceSessionId));
        System.out.println("Attempting to get user for invalidated session: " + getSessionUser(aliceSessionId));

        // Test Case 5: Verify that bob's session is still valid
        System.out.println("\n[Test Case 5: Verify bob's session is unaffected]");
        System.out.println("Is bob's session still valid? " + isSessionValid(bobSessionId));
        System.out.println("Retrieved user for bob's session: " + getSessionUser(bobSessionId));

        System.out.println("\n--- Session Management Test Finished ---");
    }
}