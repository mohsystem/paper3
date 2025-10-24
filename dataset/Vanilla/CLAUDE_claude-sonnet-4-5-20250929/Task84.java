
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

public class Task84 {
    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final int SESSION_ID_LENGTH = 32;
    private static final SecureRandom random = new SecureRandom();
    private static final Set<String> usedSessionIds = new HashSet<>();
    
    public static String generateSessionId() {
        String sessionId;
        do {
            StringBuilder sb = new StringBuilder(SESSION_ID_LENGTH);
            for (int i = 0; i < SESSION_ID_LENGTH; i++) {
                int index = random.nextInt(CHARACTERS.length());
                sb.append(CHARACTERS.charAt(index));
            }
            sessionId = sb.toString();
        } while (usedSessionIds.contains(sessionId));
        
        usedSessionIds.add(sessionId);
        return sessionId;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + generateSessionId());
        System.out.println("Test Case 2: " + generateSessionId());
        System.out.println("Test Case 3: " + generateSessionId());
        System.out.println("Test Case 4: " + generateSessionId());
        System.out.println("Test Case 5: " + generateSessionId());
    }
}
