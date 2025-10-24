
import java.security.SecureRandom;
import java.util.Base64;
import java.math.BigInteger;

public class Task128 {
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final String ALPHANUMERIC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    /**
     * Generates a cryptographically secure random integer within a range
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random integer between min and max
     */
    public static int generateSecureRandomInt(int min, int max) {
        if (min > max) {
            throw new IllegalArgumentException("min must be less than or equal to max");
        }
        int range = max - min + 1;
        return secureRandom.nextInt(range) + min;
    }
    
    /**
     * Generates a cryptographically secure random token (Base64 encoded)
     * @param byteLength Length of random bytes before encoding
     * @return Base64 encoded random token
     */
    public static String generateSecureToken(int byteLength) {
        if (byteLength <= 0) {
            throw new IllegalArgumentException("byteLength must be positive");
        }
        byte[] randomBytes = new byte[byteLength];
        secureRandom.nextBytes(randomBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }
    
    /**
     * Generates a cryptographically secure alphanumeric token
     * @param length Length of the token
     * @return Alphanumeric random token
     */
    public static String generateAlphanumericToken(int length) {
        if (length <= 0) {
            throw new IllegalArgumentException("length must be positive");
        }
        StringBuilder token = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            token.append(ALPHANUMERIC.charAt(secureRandom.nextInt(ALPHANUMERIC.length())));
        }
        return token.toString();
    }
    
    /**
     * Generates a cryptographically secure hexadecimal token
     * @param byteLength Length of random bytes
     * @return Hexadecimal random token
     */
    public static String generateHexToken(int byteLength) {
        if (byteLength <= 0) {
            throw new IllegalArgumentException("byteLength must be positive");
        }
        byte[] randomBytes = new byte[byteLength];
        secureRandom.nextBytes(randomBytes);
        return new BigInteger(1, randomBytes).toString(16);
    }
    
    /**
     * Generates a cryptographically secure random long
     * @return Random long value
     */
    public static long generateSecureRandomLong() {
        return secureRandom.nextLong();
    }
    
    public static void main(String[] args) {
        System.out.println("=== Cryptographically Secure Random Number/Token Generator ===\\n");
        
        // Test Case 1: Random integer in range
        System.out.println("Test Case 1: Random integer between 1 and 100");
        System.out.println("Result: " + generateSecureRandomInt(1, 100));
        System.out.println();
        
        // Test Case 2: Base64 encoded token (32 bytes)
        System.out.println("Test Case 2: Base64 token (32 bytes)");
        System.out.println("Result: " + generateSecureToken(32));
        System.out.println();
        
        // Test Case 3: Alphanumeric token (16 characters)
        System.out.println("Test Case 3: Alphanumeric token (16 chars)");
        System.out.println("Result: " + generateAlphanumericToken(16));
        System.out.println();
        
        // Test Case 4: Hexadecimal token (16 bytes)
        System.out.println("Test Case 4: Hexadecimal token (16 bytes)");
        System.out.println("Result: " + generateHexToken(16));
        System.out.println();
        
        // Test Case 5: Random long value
        System.out.println("Test Case 5: Random long value");
        System.out.println("Result: " + generateSecureRandomLong());
        System.out.println();
    }
}
