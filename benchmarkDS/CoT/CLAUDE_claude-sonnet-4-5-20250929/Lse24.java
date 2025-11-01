
import java.security.SecureRandom;

public class Lse24 {
    public static void generateAndCalculateDifference() {
        // Use SecureRandom for cryptographically strong random number generation
        SecureRandom secureRandom = new SecureRandom();
        
        // Generate random integers between 10 and 1000 (inclusive)
        int num1 = secureRandom.nextInt(991) + 10;
        int num2 = secureRandom.nextInt(991) + 10;
        
        // Calculate the absolute difference
        int difference = Math.abs(num1 - num2);
        
        // Print the results
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference: " + difference);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        generateAndCalculateDifference();
        System.out.println();
        
        System.out.println("Test Case 2:");
        generateAndCalculateDifference();
        System.out.println();
        
        System.out.println("Test Case 3:");
        generateAndCalculateDifference();
        System.out.println();
        
        System.out.println("Test Case 4:");
        generateAndCalculateDifference();
        System.out.println();
        
        System.out.println("Test Case 5:");
        generateAndCalculateDifference();
    }
}
