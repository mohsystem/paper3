import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.nio.charset.StandardCharsets;

public class Task135 {

    /**
     * A simple CSRF protection manager using the synchronizer token pattern.
     * In a real web application, tokens would be stored in the user's session.
     * This implementation simulates that with a map.
     */
    static class CsrfProtector {
        private static final int TOKEN_BYTE_LENGTH = 32;
        private final SecureRandom random = new SecureRandom();
        private final ConcurrentMap<String, String> sessionStore = new ConcurrentHashMap<>();

        /**
         * Generates a new, cryptographically secure token, stores it in the
         * session store, and returns the Base64-encoded token.
         *
         * @param sessionId A unique identifier for the user's session.
         * @return A Base64-encoded CSRF token.
         */
        public String generateTokenForSession(String sessionId) {
            Objects.requireNonNull(sessionId, "Session ID cannot be null");
            byte[] tokenBytes = new byte[TOKEN_BYTE_LENGTH];
            random.nextBytes(tokenBytes);
            String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
            sessionStore.put(sessionId, token);
            return token;
        }

        /**
         * Validates a submitted token against the one stored in the session.
         * This comparison is done in constant time to prevent timing attacks.
         *
         * @param sessionId      The user's session identifier.
         * @param submittedToken The token received from the client.
         * @return true if the token is valid, false otherwise.
         */
        public boolean validateToken(String sessionId, String submittedToken) {
            if (sessionId == null || submittedToken == null || sessionId.isEmpty() || submittedToken.isEmpty()) {
                return false;
            }

            String storedToken = sessionStore.get(sessionId);
            if (storedToken == null) {
                return false;
            }

            // Perform constant-time comparison
            byte[] storedTokenBytes = storedToken.getBytes(StandardCharsets.UTF_8);
            byte[] submittedTokenBytes = submittedToken.getBytes(StandardCharsets.UTF_8);

            return MessageDigest.isEqual(storedTokenBytes, submittedTokenBytes);
        }

        /**
         * Removes a token from the store, typically done on session invalidation.
         * @param sessionId The user's session identifier.
         */
        public void removeToken(String sessionId) {
            if (sessionId != null) {
                sessionStore.remove(sessionId);
            }
        }
    }

    public static void main(String[] args) {
        CsrfProtector csrfProtector = new CsrfProtector();
        String sessionId1 = "user_session_abc123";
        String sessionId2 = "user_session_def456";

        System.out.println("--- Running CSRF Protection Test Cases ---");

        // Test Case 1: Valid token
        System.out.println("\n--- Test Case 1: Valid Token ---");
        String token1 = csrfProtector.generateTokenForSession(sessionId1);
        System.out.println("Generated Token for " + sessionId1 + ": " + token1);
        boolean isValid1 = csrfProtector.validateToken(sessionId1, token1);
        System.out.println("Validation result: " + (isValid1 ? "PASS" : "FAIL"));
        assert isValid1;

        // Test Case 2: Invalid token
        System.out.println("\n--- Test Case 2: Invalid Token ---");
        String generatedToken2 = csrfProtector.generateTokenForSession(sessionId1);
        System.out.println("Generated Token for " + sessionId1 + ": " + generatedToken2);
        String wrongToken = "invalid_token_this_is_wrong_1234567890";
        boolean isValid2 = csrfProtector.validateToken(sessionId1, wrongToken);
        System.out.println("Validation with wrong token result: " + (!isValid2 ? "PASS" : "FAIL"));
        assert !isValid2;

        // Test Case 3: Missing token in session
        System.out.println("\n--- Test Case 3: Missing Token in Session ---");
        boolean isValid3 = csrfProtector.validateToken("non_existent_session", "some_token");
        System.out.println("Validation for non-existent session: " + (!isValid3 ? "PASS" : "FAIL"));
        assert !isValid3;

        // Test Case 4: Null/empty submitted token
        System.out.println("\n--- Test Case 4: Null/Empty Submitted Token ---");
        String generatedToken4 = csrfProtector.generateTokenForSession(sessionId1);
        System.out.println("Generated Token for " + sessionId1 + ": " + generatedToken4);
        boolean isValid4a = csrfProtector.validateToken(sessionId1, null);
        boolean isValid4b = csrfProtector.validateToken(sessionId1, "");
        System.out.println("Validation with null token: " + (!isValid4a ? "PASS" : "FAIL"));
        System.out.println("Validation with empty token: " + (!isValid4b ? "PASS" : "FAIL"));
        assert !isValid4a;
        assert !isValid4b;

        // Test Case 5: Token from a different session
        System.out.println("\n--- Test Case 5: Token from a Different Session ---");
        String tokenForSession1 = csrfProtector.generateTokenForSession(sessionId1);
        String tokenForSession2 = csrfProtector.generateTokenForSession(sessionId2);
        System.out.println("Token for " + sessionId1 + ": " + tokenForSession1);
        System.out.println("Token for " + sessionId2 + ": " + tokenForSession2);
        boolean isValid5 = csrfProtector.validateToken(sessionId1, tokenForSession2);
        System.out.println("Validating session 1 with session 2's token: " + (!isValid5 ? "PASS" : "FAIL"));
        assert !isValid5;

        System.out.println("\n--- All test cases completed ---");
    }
}