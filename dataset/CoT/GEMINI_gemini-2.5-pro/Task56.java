import java.security.SecureRandom;
import java.util.Base64;

public class Task56 {

    /**
     * Generates a cryptographically secure, URL-safe token.
     *
     * @param byteLength The number of random bytes to generate. 32 bytes (256 bits) is recommended.
     * @return A URL-safe Base64 encoded string token.
     */
    public static String generateToken(int byteLength) {
        if (byteLength <= 0) {
            throw new IllegalArgumentException("Byte length must be positive.");
        }
        SecureRandom secureRandom = new SecureRandom();
        byte[] tokenBytes = new byte[byteLength];
        secureRandom.nextBytes(tokenBytes);
        // Using URL-safe Base64 encoding to avoid characters like '+' and '/'
        return Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
    }

    public static void main(String[] args) {
        System.out.println("Java Token Generation Test Cases:");
        
        // Test Case 1: 16 bytes (128 bits)
        System.out.println("1. 16-byte token: " + generateToken(16));
        
        // Test Case 2: 32 bytes (256 bits) - Recommended
        System.out.println("2. 32-byte token: " + generateToken(32));
        
        // Test Case 3: 48 bytes (384 bits)
        System.out.println("3. 48-byte token: " + generateToken(48));
        
        // Test Case 4: Another 32-byte token to show it's different
        System.out.println("4. 32-byte token: " + generateToken(32));

        // Test Case 5: 64 bytes (512 bits)
        System.out.println("5. 64-byte token: " + generateToken(64));
    }
}