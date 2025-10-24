
import java.security.SecureRandom;

public class Task85 {
    private static final String ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final SecureRandom secureRandom = new SecureRandom();
    
    public static String generateRandomString(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("Length must be non-negative");
        }
        
        StringBuilder result = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = secureRandom.nextInt(ASCII_LETTERS.length());
            result.append(ASCII_LETTERS.charAt(randomIndex));
        }
        return result.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test case 1 (length 10): " + generateRandomString(10));
        System.out.println("Test case 2 (length 5): " + generateRandomString(5));
        System.out.println("Test case 3 (length 20): " + generateRandomString(20));
        System.out.println("Test case 4 (length 0): " + generateRandomString(0));
        System.out.println("Test case 5 (length 15): " + generateRandomString(15));
    }
}
