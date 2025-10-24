
import java.util.Scanner;
import java.math.BigInteger;

public class Task57 {
    public static BigInteger calculateFactorial(int n) {
        // Input validation to prevent negative numbers and handle edge cases
        if (n < 0) {
            throw new IllegalArgumentException("Factorial is not defined for negative numbers");
        }
        
        if (n == 0 || n == 1) {
            return BigInteger.ONE;
        }
        
        // Check for reasonable upper limit to prevent resource exhaustion
        if (n > 10000) {
            throw new IllegalArgumentException("Input too large - maximum value is 10000");
        }
        
        BigInteger result = BigInteger.ONE;
        for (int i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Factorial of 0 = " + calculateFactorial(0));
        System.out.println("Test Case 2: Factorial of 1 = " + calculateFactorial(1));
        System.out.println("Test Case 3: Factorial of 5 = " + calculateFactorial(5));
        System.out.println("Test Case 4: Factorial of 10 = " + calculateFactorial(10));
        System.out.println("Test Case 5: Factorial of 20 = " + calculateFactorial(20));
        
        // Test negative input handling
        try {
            System.out.println("Test Case 6 (Error): Factorial of -5");
            calculateFactorial(-5);
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
