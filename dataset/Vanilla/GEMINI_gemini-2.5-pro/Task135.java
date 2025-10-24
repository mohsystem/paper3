import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Task135 {

    // Simulates server-side session storage (SessionID -> CSRF_Token)
    private static final Map<String, String> sessionStoage = new HashMap<>();

    /**
     * Generates a unique and secure CSRF token.
     * @return A new CSRF token string.
     */
    public static String generateCsrfToken() {
        return UUID.randomUUID().toString();
    }

    /**
     * Stores the generated CSRF token in the user's session.
     * @param sessionId The user's session identifier.
     * @param token The CSRF token to store.
     */
    public static void storeTokenInSession(String sessionId, String token) {
        sessionStoage.put(sessionId, token);
    }

    /**
     * Validates the received token against the one stored in the session.
     * @param sessionId The user's session identifier.
     * @param receivedToken The token received from the client (e.g., from a form).
     * @return true if the tokens match, false otherwise.
     */
    public static boolean validateCsrfToken(String sessionId, String receivedToken) {
        if (receivedToken == null || sessionId == null) {
            return false;
        }
        String storedToken = sessionStoage.get(sessionId);
        // In a real application, use a constant-time comparison to prevent timing attacks.
        return storedToken != null && storedToken.equals(receivedToken);
    }
    
    /**
     * Simulates processing a protected action.
     * @param sessionId The user's session identifier.
     * @param receivedToken The token received from the client.
     */
    public static void processRequest(String sessionId, String receivedToken) {
        System.out.println("Processing request for session: " + sessionId + " with token: " + receivedToken);
        if (validateCsrfToken(sessionId, receivedToken)) {
            System.out.println("SUCCESS: CSRF token is valid. Action allowed.");
        } else {
            System.out.println("FAILURE: CSRF token is invalid or missing. Action rejected.");
        }
        System.out.println("-------------------------------------------------");
    }


    public static void main(String[] args) {
        // --- Test Case 1: Valid Request ---
        System.out.println("Test Case 1: Valid Request");
        String sessionId1 = "user_session_abc123";
        String token1 = generateCsrfToken();
        storeTokenInSession(sessionId1, token1);
        // User submits the form with the correct token
        processRequest(sessionId1, token1);

        // --- Test Case 2: Invalid/Incorrect Token ---
        System.out.println("Test Case 2: Invalid/Incorrect Token");
        String sessionId2 = "user_session_def456";
        String token2 = generateCsrfToken();
        storeTokenInSession(sessionId2, token2);
        // Attacker tries to submit the form with a guessed/wrong token
        processRequest(sessionId2, "wrong_token_-_");
        
        // --- Test Case 3: Missing Token ---
        System.out.println("Test Case 3: Missing Token");
        String sessionId3 = "user_session_ghi789";
        String token3 = generateCsrfToken();
        storeTokenInSession(sessionId3, token3);
        // Attacker's request is missing the token
        processRequest(sessionId3, null);

        // --- Test Case 4: No Token in Session ---
        System.out.println("Test Case 4: No Token in Session");
        String sessionId4 = "user_session_jkl012";
        // No token is generated or stored for this session
        // A request comes in with some token
        processRequest(sessionId4, "some_random_token");

        // --- Test Case 5: Token Mismatch between Users ---
        System.out.println("Test Case 5: Token Mismatch between Users");
        String userA_sessionId = "user_A_session";
        String userA_token = generateCsrfToken();
        storeTokenInSession(userA_sessionId, userA_token);
        
        String userB_sessionId = "user_B_session";
        String userB_token = generateCsrfToken();
        storeTokenInSession(userB_sessionId, userB_token);

        System.out.println("Attacker tries to use User B's token for User A's session:");
        // Attacker on a different session tries to use User B's token to attack User A
        processRequest(userA_sessionId, userB_token);
    }
}