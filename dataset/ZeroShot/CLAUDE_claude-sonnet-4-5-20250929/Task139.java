
import java.util.Scanner;

public class Task139 {
    public static double calculateCircleArea(double radius) {
        if (radius < 0) {
            throw new IllegalArgumentException("Radius cannot be negative");
        }
        return Math.PI * radius * radius;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Radius: 5");
        System.out.println("Area: " + calculateCircleArea(5));
        
        System.out.println("\\nTest Case 2 - Radius: 10.5");
        System.out.println("Area: " + calculateCircleArea(10.5));
        
        System.out.println("\\nTest Case 3 - Radius: 0");
        System.out.println("Area: " + calculateCircleArea(0));
        
        System.out.println("\\nTest Case 4 - Radius: 1");
        System.out.println("Area: " + calculateCircleArea(1));
        
        System.out.println("\\nTest Case 5 - Radius: 7.5");
        System.out.println("Area: " + calculateCircleArea(7.5));
        
        // Uncomment to test negative radius
        // System.out.println("\\nTest Case 6 - Radius: -5");
        // System.out.println("Area: " + calculateCircleArea(-5));
    }
}
