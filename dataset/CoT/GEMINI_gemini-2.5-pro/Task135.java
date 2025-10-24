import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Task135 {

    // Simulates a server-side session store, mapping session IDs to CSRF tokens.
    private static final Map<String, String> sessionStore = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();

    /**
     * Generates a cryptographically secure, random CSRF token.
     *
     * @return A Base64-encoded CSRF token string.
     */
    public static String generateCsrfToken() {
        byte[] tokenBytes = new byte[32];
        secureRandom.nextBytes(tokenBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
    }

    /**
     * Stores the CSRF token in the simulated session store for a given session ID.
     *
     * @param sessionId The user's session identifier.
     * @param token     The CSRF token to store.
     */
    public static void storeToken(String sessionId, String token) {
        sessionStore.put(sessionId, token);
    }

    /**
     * Validates a received CSRF token against the one stored in the session.
     * This uses a constant-time comparison to mitigate timing attacks.
     *
     * @param sessionId     The user's session identifier.
     * @param receivedToken The token received from the client request.
     * @return True if the token is valid, false otherwise.
     */
    public static boolean validateCsrfToken(String sessionId, String receivedToken) {
        String storedToken = sessionStore.get(sessionId);

        if (storedToken == null || receivedToken == null) {
            return false;
        }

        // Use MessageDigest.isEqual for constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(storedToken.getBytes(), receivedToken.getBytes());
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String sessionId1 = "user_session_abc123";
        String sessionId2 = "user_session_def456";

        // Test Case 1: Successful Validation
        System.out.println("--- Test Case 1: Successful Validation ---");
        String token1 = generateCsrfToken();
        storeToken(sessionId1, token1);
        System.out.println("Generated Token: " + token1);
        System.out.println("Is token valid? " + validateCsrfToken(sessionId1, token1));
        System.out.println();

        // Test Case 2: Failed Validation (Incorrect Token)
        System.out.println("--- Test Case 2: Failed Validation (Incorrect Token) ---");
        String correctToken = generateCsrfToken();
        storeToken(sessionId2, correctToken);
        String incorrectToken = "this_is_a_wrong_token";
        System.out.println("Stored Token: " + correctToken);
        System.out.println("Submitted Token: " + incorrectToken);
        System.out.println("Is token valid? " + validateCsrfToken(sessionId2, incorrectToken));
        System.out.println();

        // Test Case 3: Failed Validation (No Token in Session)
        System.out.println("--- Test Case 3: Failed Validation (No Token in Session) ---");
        String nonExistentSessionId = "non_existent_session_xyz789";
        String submittedToken = generateCsrfToken();
        System.out.println("Submitted Token: " + submittedToken);
        System.out.println("Is token for non-existent session valid? " + validateCsrfToken(nonExistentSessionId, submittedToken));
        System.out.println();
        
        // Test Case 4: Failed Validation (Submitted Token is Null)
        System.out.println("--- Test Case 4: Failed Validation (Submitted Token is Null) ---");
        System.out.println("Is null token valid? " + validateCsrfToken(sessionId1, null));
        System.out.println();

        // Test Case 5: Token Re-generation and Validation
        System.out.println("--- Test Case 5: Token Re-generation and Validation ---");
        String oldToken = sessionStore.get(sessionId1);
        String newToken = generateCsrfToken();
        storeToken(sessionId1, newToken); // Update the token for the session
        System.out.println("Old token: " + oldToken);
        System.out.println("New token: " + newToken);
        System.out.println("Is old token still valid? " + validateCsrfToken(sessionId1, oldToken));
        System.out.println("Is new token valid? " + validateCsrfToken(sessionId1, newToken));
    }
}