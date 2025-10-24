
import java.security.SecureRandom;
import java.util.Base64;

public class Task84 {
    
    public static String generateSessionID() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] randomBytes = new byte[32]; // 256 bits
        secureRandom.nextBytes(randomBytes);
        
        // Use URL-safe Base64 encoding and remove padding
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }
    
    public static void main(String[] args) {
        System.out.println("Generating 5 unique session IDs:\\n");
        
        for (int i = 1; i <= 5; i++) {
            String sessionId = generateSessionID();
            System.out.println("Session ID " + i + ": " + sessionId);
            System.out.println("Length: " + sessionId.length() + " characters\\n");
        }
    }
}
