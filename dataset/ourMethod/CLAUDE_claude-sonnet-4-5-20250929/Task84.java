
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashSet;
import java.util.Set;

public class Task84 {
    private static final int SESSION_ID_BYTES = 32;
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Set<String> usedSessionIds = new HashSet<>();
    
    /**
     * Generates a cryptographically secure random and unique session ID.
     * @return A Base64-encoded session ID string
     */
    public static String generateSessionId() {
        String sessionId;
        byte[] randomBytes = new byte[SESSION_ID_BYTES];
        
        do {
            secureRandom.nextBytes(randomBytes);
            sessionId = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
        } while (usedSessionIds.contains(sessionId));
        
        usedSessionIds.add(sessionId);
        return sessionId;
    }
    
    public static void main(String[] args) {
        System.out.println("Generating 5 unique session IDs:");
        for (int i = 1; i <= 5; i++) {
            String sessionId = generateSessionId();
            System.out.println("Session ID " + i + ": " + sessionId);
            System.out.println("Length: " + sessionId.length() + " characters");
        }
        
        System.out.println("\\nTotal unique session IDs generated: " + usedSessionIds.size());
    }
}
