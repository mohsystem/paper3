
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;

public class Task85 {
    private static final String ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final int MAX_LENGTH = 100000;
    
    public static String generateRandomString(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("Length must be non-negative");
        }
        if (length > MAX_LENGTH) {
            throw new IllegalArgumentException("Length exceeds maximum allowed: " + MAX_LENGTH);
        }
        if (length == 0) {
            return "";
        }
        
        SecureRandom secureRandom = new SecureRandom();
        StringBuilder result = new StringBuilder(length);
        
        for (int i = 0; i < length; i++) {
            int randomIndex = secureRandom.nextInt(ASCII_LETTERS.length());
            result.append(ASCII_LETTERS.charAt(randomIndex));
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 (length=10): " + generateRandomString(10));
        System.out.println("Test Case 2 (length=20): " + generateRandomString(20));
        System.out.println("Test Case 3 (length=5): " + generateRandomString(5));
        System.out.println("Test Case 4 (length=0): " + generateRandomString(0));
        System.out.println("Test Case 5 (length=15): " + generateRandomString(15));
    }
}
