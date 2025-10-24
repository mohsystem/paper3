
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

public class Task56 {
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder().withoutPadding();
    
    public static String generateToken(String username, long timestamp) {
        try {
            byte[] randomBytes = new byte[32];
            secureRandom.nextBytes(randomBytes);
            
            String data = username + ":" + timestamp + ":" + base64Encoder.encodeToString(randomBytes);
            
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(data.getBytes(StandardCharsets.UTF_8));
            
            return base64Encoder.encodeToString(hash);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        String token1 = generateToken("user123", System.currentTimeMillis());
        System.out.println("Token for user123: " + token1);
        
        System.out.println("\\nTest Case 2:");
        String token2 = generateToken("admin", System.currentTimeMillis());
        System.out.println("Token for admin: " + token2);
        
        System.out.println("\\nTest Case 3:");
        String token3 = generateToken("testuser@email.com", System.currentTimeMillis());
        System.out.println("Token for testuser@email.com: " + token3);
        
        System.out.println("\\nTest Case 4:");
        String token4 = generateToken("john_doe", 1234567890000L);
        System.out.println("Token for john_doe with fixed timestamp: " + token4);
        
        System.out.println("\\nTest Case 5:");
        String token5 = generateToken("guest", System.currentTimeMillis());
        System.out.println("Token for guest: " + token5);
    }
}
