import java.security.SecureRandom;
import java.util.Formatter;

public class Task84 {

    private static final int SESSION_ID_BYTES = 16; // 128 bits for a strong session ID

    /**
     * Generates a cryptographically secure, random and unique session ID.
     * The ID is returned as a hexadecimal string.
     *
     * @return A random 32-character hexadecimal session ID string, or an empty string on failure.
     */
    public static String generateSessionId() {
        try {
            SecureRandom random = new SecureRandom();
            byte[] sessionIdBytes = new byte[SESSION_ID_BYTES];
            random.nextBytes(sessionIdBytes);

            // Convert byte array to a hexadecimal string for URL-safe transport
            StringBuilder sb = new StringBuilder(sessionIdBytes.length * 2);
            try (Formatter formatter = new Formatter(sb)) {
                for (byte b : sessionIdBytes) {
                    formatter.format("%02x", b);
                }
            }
            return sb.toString();
        } catch (Exception e) {
            // In a real application, this should be logged properly.
            System.err.println("Error generating session ID: " + e.getMessage());
            return "";
        }
    }

    public static void main(String[] args) {
        System.out.println("Java: Generating 5 unique session IDs");
        for (int i = 0; i < 5; i++) {
            String sessionId = generateSessionId();
            System.out.println("Session ID " + (i + 1) + ": " + sessionId);
        }
    }
}