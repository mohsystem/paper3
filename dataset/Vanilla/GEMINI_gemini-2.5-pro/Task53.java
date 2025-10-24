import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

class Task53 {
    private final Map<String, String> activeSessions;

    public Task53() {
        this.activeSessions = new HashMap<>();
    }

    /**
     * Creates a new session for a given user.
     * @param username The username for whom the session is created.
     * @return The unique session ID.
     */
    public String createSession(String username) {
        String sessionId = UUID.randomUUID().toString();
        activeSessions.put(sessionId, username);
        System.out.println("Session created for " + username + " with ID: " + sessionId);
        return sessionId;
    }

    /**
     * Retrieves the username associated with a session ID.
     * @param sessionId The session ID to look up.
     * @return The username, or null if the session is not found.
     */
    public String getUserFromSession(String sessionId) {
        return activeSessions.get(sessionId);
    }

    /**
     * Ends an active session.
     * @param sessionId The session ID to terminate.
     */
    public void endSession(String sessionId) {
        if (activeSessions.containsKey(sessionId)) {
            String username = activeSessions.remove(sessionId);
            System.out.println("Session ended for " + username + " with ID: " + sessionId);
        } else {
            System.out.println("Session with ID " + sessionId + " not found.");
        }
    }

    /**
     * Checks if a session is currently active.
     * @param sessionId The session ID to check.
     * @return true if the session is active, false otherwise.
     */
    public boolean isSessionActive(String sessionId) {
        return activeSessions.containsKey(sessionId);
    }

    public static void main(String[] args) {
        Task53 sessionManager = new Task53();
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Create a session for user 'alice'
        System.out.println("\n--- Test Case 1: Create Session ---");
        String aliceSessionId = sessionManager.createSession("alice");

        // Test Case 2: Check if alice's session is active
        System.out.println("\n--- Test Case 2: Check Active Session ---");
        System.out.println("Is alice's session active? " + sessionManager.isSessionActive(aliceSessionId));

        // Test Case 3: Get user from session ID
        System.out.println("\n--- Test Case 3: Get User from Session ---");
        String username = sessionManager.getUserFromSession(aliceSessionId);
        System.out.println("User for session " + aliceSessionId + " is: " + username);
        
        // Test Case 4: Create another session for 'bob' and end 'alice's session
        System.out.println("\n--- Test Case 4: End Session ---");
        String bobSessionId = sessionManager.createSession("bob");
        sessionManager.endSession(aliceSessionId);
        
        // Test Case 5: Check if alice's session is still active and bob's is active
        System.out.println("\n--- Test Case 5: Verify Session Status ---");
        System.out.println("Is alice's session active after ending? " + sessionManager.isSessionActive(aliceSessionId));
        System.out.println("Is bob's session active? " + sessionManager.isSessionActive(bobSessionId));
        String nonExistentUser = sessionManager.getUserFromSession(aliceSessionId);
        System.out.println("Attempting to get user from ended session: " + nonExistentUser);
    }
}