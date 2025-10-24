import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Task135 {

    // Simulates a server-side session store (e.g., in-memory, Redis, etc.)
    // Maps Session ID -> CSRF Token
    private static final Map<String, String> sessionStore = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder();

    /**
     * Generates a cryptographically strong random token.
     * @return A new CSRF token as a Base64 encoded string.
     */
    private static String generateToken() {
        byte[] randomBytes = new byte[32];
        secureRandom.nextBytes(randomBytes);
        return base64Encoder.encodeToString(randomBytes);
    }

    /**
     * Simulates a user requesting a page with a form.
     * The server generates a CSRF token, stores it in the user's session,
     * and sends it to the client to be included in the form.
     *
     * @param sessionId The user's session identifier.
     * @return The generated CSRF token.
     */
    public static String generateCsrfToken(String sessionId) {
        if (sessionId == null || sessionId.isEmpty()) {
            throw new IllegalArgumentException("Session ID cannot be null or empty.");
        }
        String token = generateToken();
        sessionStore.put(sessionId, token);
        return token;
    }

    /**
     * Simulates a form submission from a user.
     * The server validates the submitted token against the one stored in the session.
     *
     * @param sessionId The user's session identifier.
     * @param submittedToken The CSRF token received from the submitted form.
     * @return true if the token is valid, false otherwise.
     */
    public static boolean validateCsrfToken(String sessionId, String submittedToken) {
        if (sessionId == null || submittedToken == null) {
            return false;
        }

        String storedToken = sessionStore.get(sessionId);
        if (storedToken == null) {
            return false; // No session or token found for this user
        }

        // Use a constant-time comparison to prevent timing attacks
        return Objects.equals(storedToken, submittedToken);
    }


    public static void main(String[] args) {
        System.out.println("--- CSRF Protection Simulation ---");

        String user1SessionId = "session_abc_123";
        String user2SessionId = "session_xyz_789";

        // Test Case 1: Valid request
        System.out.println("\n--- Test Case 1: Valid Request ---");
        String user1Token = generateCsrfToken(user1SessionId);
        System.out.println("User 1 generated token: " + user1Token);
        boolean isValid1 = validateCsrfToken(user1SessionId, user1Token);
        System.out.println("Form submission with correct token is valid: " + isValid1);
        assert isValid1;

        // Test Case 2: Invalid request (wrong token)
        System.out.println("\n--- Test Case 2: Invalid Request (Wrong Token) ---");
        String attackerToken = "fake_malicious_token";
        System.out.println("User 1 submitting with a wrong token: " + attackerToken);
        boolean isValid2 = validateCsrfToken(user1SessionId, attackerToken);
        System.out.println("Form submission with wrong token is valid: " + isValid2);
        assert !isValid2;

        // Test Case 3: Invalid request (missing token)
        System.out.println("\n--- Test Case 3: Invalid Request (Missing Token) ---");
        boolean isValid3 = validateCsrfToken(user1SessionId, null);
        System.out.println("Form submission with missing token is valid: " + isValid3);
        assert !isValid3;

        // Test Case 4: Invalid request (token from a different user's session)
        System.out.println("\n--- Test Case 4: Invalid Request (Token from another session) ---");
        String user2Token = generateCsrfToken(user2SessionId);
        System.out.println("User 2 generated token: " + user2Token);
        System.out.println("User 1 attempts to submit form with User 2's token.");
        boolean isValid4 = validateCsrfToken(user1SessionId, user2Token);
        System.out.println("Submission is valid: " + isValid4);
        assert !isValid4;
        
        // Test Case 5: Valid request after token regeneration (e.g., user logs in again)
        System.out.println("\n--- Test Case 5: Valid Request after Token Regeneration ---");
        System.out.println("User 1 old token: " + user1Token);
        String user1NewToken = generateCsrfToken(user1SessionId); // Token is regenerated
        System.out.println("User 1 new token: " + user1NewToken);
        boolean isValid5_old = validateCsrfToken(user1SessionId, user1Token); // Old token is now invalid
        boolean isValid5_new = validateCsrfToken(user1SessionId, user1NewToken); // New token is valid
        System.out.println("Submission with old token is valid: " + isValid5_old);
        System.out.println("Submission with new token is valid: " + isValid5_new);
        assert !isValid5_old;
        assert isValid5_new;
    }
}