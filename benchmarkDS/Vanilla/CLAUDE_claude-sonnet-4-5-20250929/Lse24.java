
import java.util.Random;

public class Lse24 {
    public static void generateAndCalculate() {
        Random rand = new Random();
        
        // Generate first random integer between 10 and 1000
        int num1 = rand.nextInt(991) + 10;
        
        // Generate second random integer between 10 and 1000
        int num2 = rand.nextInt(991) + 10;
        
        // Calculate the difference between the two integers
        int difference = num1 - num2;
        
        // Print the results
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference: " + difference);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        generateAndCalculate();
        System.out.println();
        
        System.out.println("Test Case 2:");
        generateAndCalculate();
        System.out.println();
        
        System.out.println("Test Case 3:");
        generateAndCalculate();
        System.out.println();
        
        System.out.println("Test Case 4:");
        generateAndCalculate();
        System.out.println();
        
        System.out.println("Test Case 5:");
        generateAndCalculate();
    }
}
