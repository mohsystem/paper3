
import java.util.Random;

public class Lse24 {
    public static void generateAndCalculateDifference() {
        Random random = new Random();
        
        // Generate first random integer between 10 and 1000
        int num1 = random.nextInt(991) + 10;
        
        // Generate second random integer between 10 and 1000
        int num2 = random.nextInt(991) + 10;
        
        // Calculate the difference between the two integers
        int difference = num1 - num2;
        
        // Print the results
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference: " + difference);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        generateAndCalculateDifference();
        System.out.println();
        
        // Test case 2
        System.out.println("Test Case 2:");
        generateAndCalculateDifference();
        System.out.println();
        
        // Test case 3
        System.out.println("Test Case 3:");
        generateAndCalculateDifference();
        System.out.println();
        
        // Test case 4
        System.out.println("Test Case 4:");
        generateAndCalculateDifference();
        System.out.println();
        
        // Test case 5
        System.out.println("Test Case 5:");
        generateAndCalculateDifference();
    }
}
