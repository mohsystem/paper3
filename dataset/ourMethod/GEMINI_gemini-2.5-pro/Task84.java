import java.security.SecureRandom;
import java.util.Base64;

public class Task84 {

    private static final int SESSION_ID_BYTES = 32;

    /**
     * Generates a cryptographically secure, random and unique session ID.
     * The ID is generated from 32 random bytes and encoded using URL-safe Base64.
     *
     * @return A URL-safe Base64 encoded string representing the session ID.
     */
    public static String generateSessionId() {
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[SESSION_ID_BYTES];
        random.nextBytes(bytes);
        // Using URL-safe Base64 encoder without padding for a clean, URL-friendly ID
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    public static void main(String[] args) {
        System.out.println("Generating 5 random and unique session IDs:");
        for (int i = 0; i < 5; i++) {
            String sessionId = generateSessionId();
            System.out.println("Session ID " + (i + 1) + ": " + sessionId);
            if (sessionId == null || sessionId.isEmpty()) {
                System.err.println("Test case " + (i + 1) + " failed: Generated session ID is null or empty.");
            }
        }
    }
}