
import java.util.Scanner;
import java.math.BigInteger;

public class Task142 {
    /**
     * Computes the greatest common divisor of two numbers using Euclidean algorithm.
     * @param a First number
     * @param b Second number
     * @return GCD of a and b
     * @throws IllegalArgumentException if either number is negative
     */
    public static long gcd(long a, long b) {
        // Validate inputs - only non-negative numbers allowed
        if (a < 0 || b < 0) {
            throw new IllegalArgumentException("Both numbers must be non-negative");
        }
        
        // Handle edge case where both are zero
        if (a == 0 && b == 0) {
            throw new IllegalArgumentException("GCD of 0 and 0 is undefined");
        }
        
        // Euclidean algorithm
        while (b != 0) {
            long temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: GCD(48, 18) = " + gcd(48L, 18L));
        System.out.println("Test Case 2: GCD(100, 50) = " + gcd(100L, 50L));
        System.out.println("Test Case 3: GCD(17, 19) = " + gcd(17L, 19L));
        System.out.println("Test Case 4: GCD(0, 5) = " + gcd(0L, 5L));
        System.out.println("Test Case 5: GCD(1071, 462) = " + gcd(1071L, 462L));
        
        // Interactive input with validation
        try (Scanner scanner = new Scanner(System.in)) {
            System.out.println("\\nEnter two non-negative integers:");
            System.out.print("First number: ");
            
            if (!scanner.hasNextLong()) {
                System.err.println("Error: Invalid input. Please enter a valid integer.");
                return;
            }
            long num1 = scanner.nextLong();
            
            System.out.print("Second number: ");
            if (!scanner.hasNextLong()) {
                System.err.println("Error: Invalid input. Please enter a valid integer.");
                return;
            }
            long num2 = scanner.nextLong();
            
            try {
                long result = gcd(num1, num2);
                System.out.println("GCD(" + num1 + ", " + num2 + ") = " + result);
            } catch (IllegalArgumentException e) {
                System.err.println("Error: " + e.getMessage());
            }
        } catch (Exception e) {
            System.err.println("Error: An unexpected error occurred.");
        }
    }
}
