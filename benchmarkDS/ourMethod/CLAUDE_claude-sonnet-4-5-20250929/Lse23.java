
import java.security.SecureRandom;

public class Lse23 {
    public static void generateAndSum() {
        // Use SecureRandom for cryptographically secure random number generation
        SecureRandom secureRandom = new SecureRandom();
        
        // Generate three random integers larger than 1000
        // Range: 1001 to Integer.MAX_VALUE to avoid overflow in addition
        // Using nextInt with bound to ensure values are within safe range
        int a = 1001 + secureRandom.nextInt(Integer.MAX_VALUE / 3 - 1001);
        int b = 1001 + secureRandom.nextInt(Integer.MAX_VALUE / 3 - 1001);
        int c = 1001 + secureRandom.nextInt(Integer.MAX_VALUE / 3 - 1001);
        
        // Print the three integers
        System.out.println("a : " + a);
        System.out.println("b : " + b);
        System.out.println("c : " + c);
        
        // Check for overflow before addition
        long sum = (long) a + (long) b + (long) c;
        if (sum > Integer.MAX_VALUE) {
            System.err.println("Sum overflow detected");
            return;
        }
        
        // Add the three integers and print the result
        int sumInt = a + b + c;
        System.out.println("sum: " + sumInt);
        System.out.println("sum2: " + sumInt);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test case 1:");
        generateAndSum();
        System.out.println();
        
        // Test case 2
        System.out.println("Test case 2:");
        generateAndSum();
        System.out.println();
        
        // Test case 3
        System.out.println("Test case 3:");
        generateAndSum();
        System.out.println();
        
        // Test case 4
        System.out.println("Test case 4:");
        generateAndSum();
        System.out.println();
        
        // Test case 5
        System.out.println("Test case 5:");
        generateAndSum();
    }
}
