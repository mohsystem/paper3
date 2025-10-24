
import java.security.SecureRandom;

public class Task85 {
    private static final String ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final SecureRandom secureRandom = new SecureRandom();
    
    public static String generateRandomString(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("Length cannot be negative");
        }
        if (length == 0) {
            return "";
        }
        if (length > 1000000) {
            throw new IllegalArgumentException("Length too large to prevent resource exhaustion");
        }
        
        StringBuilder result = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = secureRandom.nextInt(ASCII_LETTERS.length());
            result.append(ASCII_LETTERS.charAt(randomIndex));
        }
        return result.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 (length=10): " + generateRandomString(10));
        System.out.println("Test Case 2 (length=5): " + generateRandomString(5));
        System.out.println("Test Case 3 (length=20): " + generateRandomString(20));
        System.out.println("Test Case 4 (length=1): " + generateRandomString(1));
        System.out.println("Test Case 5 (length=0): '" + generateRandomString(0) + "'");
    }
}
