
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.time.Instant;

public class Task56 {
    
    public static String generateToken(String userId) {
        try {
            SecureRandom secureRandom = new SecureRandom();
            byte[] randomBytes = new byte[32];
            secureRandom.nextBytes(randomBytes);
            
            String timestamp = String.valueOf(Instant.now().toEpochMilli());
            String dataToHash = userId + timestamp + Base64.getEncoder().encodeToString(randomBytes);
            
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(dataToHash.getBytes(StandardCharsets.UTF_8));
            
            return Base64.getUrlEncoder().withoutPadding().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error generating token", e);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        String token1 = generateToken("user123");
        System.out.println("Token for user123: " + token1);
        
        System.out.println("\\nTest Case 2:");
        String token2 = generateToken("admin@example.com");
        System.out.println("Token for admin@example.com: " + token2);
        
        System.out.println("\\nTest Case 3:");
        String token3 = generateToken("testUser456");
        System.out.println("Token for testUser456: " + token3);
        
        System.out.println("\\nTest Case 4:");
        String token4 = generateToken("john.doe");
        System.out.println("Token for john.doe: " + token4);
        
        System.out.println("\\nTest Case 5:");
        String token5 = generateToken("user123");
        System.out.println("Token for user123 (second generation): " + token5);
        System.out.println("Tokens are unique: " + (!token1.equals(token5)));
    }
}
