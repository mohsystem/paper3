
import java.security.SecureRandom;

public class Lse23 {
    public static void generateAndSum() {
        SecureRandom secureRandom = new SecureRandom();
        
        // Generate three random integers larger than 1000
        int a = secureRandom.nextInt(Integer.MAX_VALUE - 1001) + 1001;
        int b = secureRandom.nextInt(Integer.MAX_VALUE - 1001) + 1001;
        int c = secureRandom.nextInt(Integer.MAX_VALUE - 1001) + 1001;
        
        // Print the three integers
        System.out.println("a : " + a);
        System.out.println("b : " + b);
        System.out.println("c : " + c);
        
        // Add the three integers and print the result
        long sum = (long)a + (long)b + (long)c;
        long sum2 = (long)a + (long)b + (long)c;
        
        System.out.println("sum: " + sum);
        System.out.println("sum2: " + sum2);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        generateAndSum();
        System.out.println("\\nTest Case 2:");
        generateAndSum();
        System.out.println("\\nTest Case 3:");
        generateAndSum();
        System.out.println("\\nTest Case 4:");
        generateAndSum();
        System.out.println("\\nTest Case 5:");
        generateAndSum();
    }
}
