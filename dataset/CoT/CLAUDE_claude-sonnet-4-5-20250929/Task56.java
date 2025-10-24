
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;
import java.time.Instant;
import java.nio.charset.StandardCharsets;

public class Task56 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final int TOKEN_LENGTH = 32;
    
    public static String generateToken(String userId, long timestamp) {
        try {
            // Generate random bytes using SecureRandom
            byte[] randomBytes = new byte[TOKEN_LENGTH];
            SECURE_RANDOM.nextBytes(randomBytes);
            
            // Combine userId, timestamp, and random bytes
            String combinedData = userId + ":" + timestamp + ":" + Base64.getEncoder().encodeToString(randomBytes);
            
            // Hash the combined data using SHA-256
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(combinedData.getBytes(StandardCharsets.UTF_8));
            
            // Encode to Base64 for safe transmission
            return Base64.getUrlEncoder().withoutPadding().encodeToString(hash);
            
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    public static String generateToken(String userId) {
        return generateToken(userId, Instant.now().toEpochMilli());
    }
    
    public static void main(String[] args) {
        // Test case 1: Generate token for user1
        String token1 = generateToken("user1");
        System.out.println("Test 1 - Token for user1: " + token1);
        
        // Test case 2: Generate token for user2
        String token2 = generateToken("user2");
        System.out.println("Test 2 - Token for user2: " + token2);
        
        // Test case 3: Generate token with specific timestamp
        long specificTimestamp = 1700000000000L;
        String token3 = generateToken("user3", specificTimestamp);
        System.out.println("Test 3 - Token for user3 with timestamp: " + token3);
        
        // Test case 4: Verify tokens are unique for same user
        String token4a = generateToken("user4");
        try {
            Thread.sleep(10); // Small delay to ensure different timestamp
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        String token4b = generateToken("user4");
        System.out.println("Test 4 - Two tokens for user4 are different: " + !token4a.equals(token4b));
        
        // Test case 5: Generate token for admin user
        String token5 = generateToken("admin");
        System.out.println("Test 5 - Token for admin: " + token5);
    }
}
