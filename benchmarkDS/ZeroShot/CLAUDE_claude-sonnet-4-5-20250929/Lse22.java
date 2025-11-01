
import java.security.SecureRandom;

public class Lse22 {
    public static int generateRandomNumber() {
        SecureRandom secureRandom = new SecureRandom();
        // Generate a random number larger than 1000
        // Using nextInt with bound to get a number between 1001 and 1001 + Integer.MAX_VALUE/2
        int randomNumber = 1001 + secureRandom.nextInt(Integer.MAX_VALUE - 1001);
        return randomNumber;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int result1 = generateRandomNumber();
        System.out.println("Test 1 - Random number: " + result1 + " (Should be > 1000)");
        
        // Test case 2
        int result2 = generateRandomNumber();
        System.out.println("Test 2 - Random number: " + result2 + " (Should be > 1000)");
        
        // Test case 3
        int result3 = generateRandomNumber();
        System.out.println("Test 3 - Random number: " + result3 + " (Should be > 1000)");
        
        // Test case 4
        int result4 = generateRandomNumber();
        System.out.println("Test 4 - Random number: " + result4 + " (Should be > 1000)");
        
        // Test case 5
        int result5 = generateRandomNumber();
        System.out.println("Test 5 - Random number: " + result5 + " (Should be > 1000)");
    }
}
