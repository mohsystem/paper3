
import java.util.Scanner;

public class Task139 {
    private static final double PI = 3.14159265358979323846;
    
    public static double calculateCircleArea(double radius) {
        // Input validation
        if (radius < 0) {
            throw new IllegalArgumentException("Radius cannot be negative");
        }
        if (Double.isNaN(radius) || Double.isInfinite(radius)) {
            throw new IllegalArgumentException("Invalid radius value");
        }
        
        // Calculate area with overflow check
        double area = PI * radius * radius;
        
        if (Double.isInfinite(area)) {
            throw new ArithmeticException("Radius too large, calculation overflow");
        }
        
        return area;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Radius: 5.0");
        System.out.println("Area: " + calculateCircleArea(5.0));
        
        System.out.println("\\nTest Case 2 - Radius: 10.5");
        System.out.println("Area: " + calculateCircleArea(10.5));
        
        System.out.println("\\nTest Case 3 - Radius: 0.0");
        System.out.println("Area: " + calculateCircleArea(0.0));
        
        System.out.println("\\nTest Case 4 - Radius: 1.0");
        System.out.println("Area: " + calculateCircleArea(1.0));
        
        System.out.println("\\nTest Case 5 - Radius: 7.5");
        System.out.println("Area: " + calculateCircleArea(7.5));
        
        // Test error case
        try {
            System.out.println("\\nTest Case 6 - Radius: -5.0 (Error case)");
            System.out.println("Area: " + calculateCircleArea(-5.0));
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
