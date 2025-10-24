
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task128 {
    private static final SecureRandom secureRandom = new SecureRandom();
    
    /**
     * Generates a cryptographically secure random integer within the specified range.
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random integer between min and max
     */
    public static int generateRandomNumber(int min, int max) {
        if (min > max) {
            throw new IllegalArgumentException("min must be less than or equal to max");
        }
        if (min == max) {
            return min;
        }
        
        long range = (long) max - (long) min + 1;
        if (range <= 0) {
            throw new IllegalArgumentException("Range too large");
        }
        
        // Generate unbiased random number in range
        int randomInt = secureRandom.nextInt((int) range);
        return min + randomInt;
    }
    
    /**
     * Generates a cryptographically secure random token.
     * @param length Length of the token in bytes
     * @return Base64-encoded random token
     */
    public static String generateRandomToken(int length) {
        if (length <= 0 || length > 1024) {
            throw new IllegalArgumentException("Length must be between 1 and 1024 bytes");
        }
        
        byte[] tokenBytes = new byte[length];
        secureRandom.nextBytes(tokenBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
    }
    
    /**
     * Generates a cryptographically secure random hexadecimal token.
     * @param length Length of the token in bytes
     * @return Hexadecimal string representation of random bytes
     */
    public static String generateRandomHexToken(int length) {
        if (length <= 0 || length > 1024) {
            throw new IllegalArgumentException("Length must be between 1 and 1024 bytes");
        }
        
        byte[] tokenBytes = new byte[length];
        secureRandom.nextBytes(tokenBytes);
        
        StringBuilder hexString = new StringBuilder(length * 2);
        for (byte b : tokenBytes) {
            hexString.append(String.format("%02x", b & 0xFF));
        }
        return hexString.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Generate random number in range 1-100
        System.out.println("Test 1 - Random number (1-100): " + generateRandomNumber(1, 100));
        
        // Test case 2: Generate random number in range 0-1000
        System.out.println("Test 2 - Random number (0-1000): " + generateRandomNumber(0, 1000));
        
        // Test case 3: Generate 16-byte Base64 token
        System.out.println("Test 3 - 16-byte Base64 token: " + generateRandomToken(16));
        
        // Test case 4: Generate 32-byte Base64 token
        System.out.println("Test 4 - 32-byte Base64 token: " + generateRandomToken(32));
        
        // Test case 5: Generate 16-byte hexadecimal token
        System.out.println("Test 5 - 16-byte hex token: " + generateRandomHexToken(16));
    }
}
