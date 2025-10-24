
import java.security.SecureRandom;
import java.util.Base64;

public class Task128 {
    private static final SecureRandom secureRandom = new SecureRandom();
    
    // Generate random integer within a range
    public static int generateRandomInt(int min, int max) {
        if (min >= max) {
            throw new IllegalArgumentException("Max must be greater than min");
        }
        return secureRandom.nextInt(max - min) + min;
    }
    
    // Generate random long
    public static long generateRandomLong() {
        return secureRandom.nextLong();
    }
    
    // Generate random token (Base64 encoded)
    public static String generateRandomToken(int byteLength) {
        if (byteLength <= 0) {
            throw new IllegalArgumentException("Byte length must be positive");
        }
        byte[] randomBytes = new byte[byteLength];
        secureRandom.nextBytes(randomBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }
    
    // Generate random alphanumeric string
    public static String generateRandomAlphanumeric(int length) {
        if (length <= 0) {
            throw new IllegalArgumentException("Length must be positive");
        }
        String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append(chars.charAt(secureRandom.nextInt(chars.length())));
        }
        return sb.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Random Integer (1-100):");
        System.out.println(generateRandomInt(1, 100));
        
        System.out.println("\\nTest Case 2 - Random Long:");
        System.out.println(generateRandomLong());
        
        System.out.println("\\nTest Case 3 - Random Token (32 bytes):");
        System.out.println(generateRandomToken(32));
        
        System.out.println("\\nTest Case 4 - Random Alphanumeric (16 chars):");
        System.out.println(generateRandomAlphanumeric(16));
        
        System.out.println("\\nTest Case 5 - Multiple Random Integers:");
        for (int i = 0; i < 5; i++) {
            System.out.println(generateRandomInt(1, 1000));
        }
    }
}
