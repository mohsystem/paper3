
import java.util.Random;

public class Lse23 {
    public static void generateAndPrintRandomIntegers() {
        Random random = new Random();
        
        // Generate three random integers larger than 1000
        int a = random.nextInt(9000) + 1001; // generates numbers from 1001 to 10000
        int b = random.nextInt(9000) + 1001;
        int c = random.nextInt(9000) + 1001;
        
        // Print the three integers
        System.out.println("a : " + a);
        System.out.println("b : " + b);
        System.out.println("c : " + c);
        
        // Add the three integers and print the result
        int sum = a + b + c;
        System.out.println("sum: " + sum);
        
        // Print sum2 (alternative calculation)
        int sum2 = a + b + c;
        System.out.println("sum2: " + sum2);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        generateAndPrintRandomIntegers();
        System.out.println();
        
        System.out.println("Test Case 2:");
        generateAndPrintRandomIntegers();
        System.out.println();
        
        System.out.println("Test Case 3:");
        generateAndPrintRandomIntegers();
        System.out.println();
        
        System.out.println("Test Case 4:");
        generateAndPrintRandomIntegers();
        System.out.println();
        
        System.out.println("Test Case 5:");
        generateAndPrintRandomIntegers();
    }
}
