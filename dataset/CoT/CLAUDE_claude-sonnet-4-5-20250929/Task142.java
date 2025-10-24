
import java.util.Scanner;

public class Task142 {
    // Function to find GCD using Euclidean algorithm
    public static long findGCD(long a, long b) {
        // Input validation - convert negative numbers to positive
        a = Math.abs(a);
        b = Math.abs(b);
        
        // Handle edge case where both numbers are 0
        if (a == 0 && b == 0) {
            throw new IllegalArgumentException("GCD is undefined for both numbers being zero");
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
        // Test case 1: Normal positive numbers
        System.out.println("Test 1 - GCD(48, 18): " + findGCD(48, 18));
        
        // Test case 2: One number is 0
        System.out.println("Test 2 - GCD(0, 5): " + findGCD(0, 5));
        
        // Test case 3: Same numbers
        System.out.println("Test 3 - GCD(100, 100): " + findGCD(100, 100));
        
        // Test case 4: Prime numbers (coprime)
        System.out.println("Test 4 - GCD(17, 19): " + findGCD(17, 19));
        
        // Test case 5: Large numbers with negative input
        System.out.println("Test 5 - GCD(-270, 192): " + findGCD(-270, 192));
    }
}
