
import java.util.Scanner;
import java.math.BigInteger;

public class Task57 {
    /**
     * Calculates the factorial of a non-negative integer.
     * @param n The number to calculate factorial for
     * @return The factorial as a BigInteger
     * @throws IllegalArgumentException if n is negative or exceeds safe limit
     */
    public static BigInteger factorial(int n) {
        if (n < 0) {
            throw new IllegalArgumentException("Input must be non-negative");
        }
        if (n > 100000) {
            throw new IllegalArgumentException("Input too large (max 100000)");
        }
        
        BigInteger result = BigInteger.ONE;
        for (int i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: 0! = 1
        try {
            System.out.println("0! = " + factorial(0));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 2: 5! = 120
        try {
            System.out.println("5! = " + factorial(5));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 3: 10! = 3628800
        try {
            System.out.println("10! = " + factorial(10));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 4: 20! = 2432902008176640000
        try {
            System.out.println("20! = " + factorial(20));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 5: Negative number (should throw exception)
        try {
            System.out.println("-5! = " + factorial(-5));
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
