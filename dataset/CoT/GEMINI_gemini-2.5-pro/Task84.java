import java.util.UUID;
import java.security.SecureRandom;
import java.util.Base64;

public class Task84 {

    /**
     * Generates a random and unique session ID using Java's built-in UUID class.
     * UUID Version 4 is cryptographically secure and suitable for session IDs.
     *
     * @return A unique session ID string.
     */
    public static String generateSessionId() {
        return UUID.randomUUID().toString();
    }
    
    /**
     * An alternative method to generate a session ID using SecureRandom.
     * This provides more control over the length and format.
     *
     * @param length The number of random bytes to generate.
     * @return A URL-safe Base64 encoded session ID string.
     */
    public static String generateSecureSessionId(int length) {
        SecureRandom secureRandom = new SecureRandom();
        byte[] randomBytes = new byte[length];
        secureRandom.nextBytes(randomBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }

    public static void main(String[] args) {
        System.out.println("--- Testing generateSessionId() [UUID] ---");
        for (int i = 0; i < 5; i++) {
            String sessionId = generateSessionId();
            System.out.println("Test Case " + (i + 1) + ": " + sessionId);
        }

        System.out.println("\n--- Testing generateSecureSessionId(24) [SecureRandom] ---");
        for (int i = 0; i < 5; i++) {
            String sessionId = generateSecureSessionId(24); // 24 bytes = 192 bits
            System.out.println("Test Case " + (i + 1) + ": " + sessionId);
        }
    }
}