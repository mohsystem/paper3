
import java.security.SecureRandom;

public class Lse22 {
    /**
     * Generates a cryptographically secure random number larger than 1000.
     * Uses SecureRandom instead of Random for unpredictable number generation.
     * 
     * @return A random integer greater than 1000
     */
    public static int generateSecureRandomNumber() {
        // Use SecureRandom for cryptographically strong random numbers
        // This prevents predictable number generation attacks
        SecureRandom secureRandom = new SecureRandom();
        
        // Generate a random number in range [0, Integer.MAX_VALUE - 1001)
        // Add 1001 to ensure result is always > 1000
        // Using nextInt with bound prevents overflow and ensures positive result
        int randomValue = secureRandom.nextInt(Integer.MAX_VALUE - 1001);
        
        return randomValue + 1001;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int result1 = generateSecureRandomNumber();
        System.out.println("Test 1 - Random number: " + result1 + " (should be > 1000)");
        
        // Test case 2
        int result2 = generateSecureRandomNumber();
        System.out.println("Test 2 - Random number: " + result2 + " (should be > 1000)");
        
        // Test case 3
        int result3 = generateSecureRandomNumber();
        System.out.println("Test 3 - Random number: " + result3 + " (should be > 1000)");
        
        // Test case 4
        int result4 = generateSecureRandomNumber();
        System.out.println("Test 4 - Random number: " + result4 + " (should be > 1000)");
        
        // Test case 5
        int result5 = generateSecureRandomNumber();
        System.out.println("Test 5 - Random number: " + result5 + " (should be > 1000)");
    }
}
