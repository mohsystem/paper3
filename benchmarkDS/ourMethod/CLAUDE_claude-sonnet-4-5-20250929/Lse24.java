
import java.security.SecureRandom;

public class Lse24 {
    // Use SecureRandom for cryptographically strong random number generation
    // This prevents predictable values that could be exploited
    private static final SecureRandom secureRandom = new SecureRandom();
    
    public static int[] generateAndCalculateDifference() {
        // Generate random integers between 10 and 1000 (inclusive)
        // Using SecureRandom instead of Math.random() for better unpredictability
        int num1 = secureRandom.nextInt(991) + 10; // 991 + 10 = range [10, 1000]
        int num2 = secureRandom.nextInt(991) + 10;
        
        // Calculate absolute difference to avoid negative results
        int difference = Math.abs(num1 - num2);
        
        return new int[]{num1, num2, difference};
    }
    
    public static void printResults(int num1, int num2, int difference) {
        // Use constant format string to prevent format string injection
        // Validate that inputs are within expected range before printing
        if (num1 < 10 || num1 > 1000 || num2 < 10 || num2 > 1000) {
            System.err.println("Invalid input values");
            return;
        }
        
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference: " + difference);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        int[] result1 = generateAndCalculateDifference();
        printResults(result1[0], result1[1], result1[2]);
        System.out.println();
        
        // Test case 2
        System.out.println("Test Case 2:");
        int[] result2 = generateAndCalculateDifference();
        printResults(result2[0], result2[1], result2[2]);
        System.out.println();
        
        // Test case 3
        System.out.println("Test Case 3:");
        int[] result3 = generateAndCalculateDifference();
        printResults(result3[0], result3[1], result3[2]);
        System.out.println();
        
        // Test case 4
        System.out.println("Test Case 4:");
        int[] result4 = generateAndCalculateDifference();
        printResults(result4[0], result4[1], result4[2]);
        System.out.println();
        
        // Test case 5
        System.out.println("Test Case 5:");
        int[] result5 = generateAndCalculateDifference();
        printResults(result5[0], result5[1], result5[2]);
    }
}
