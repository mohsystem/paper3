
import java.util.Scanner;
import java.nio.charset.StandardCharsets;

public class Task139 {
    private static final double PI = 3.14159265358979323846;
    private static final double MAX_RADIUS = 1e308;
    private static final double MIN_RADIUS = 0.0;

    public static double calculateCircleArea(double radius) {
        // Validate input: radius must be non-negative and within valid range
        if (Double.isNaN(radius) || Double.isInfinite(radius)) {
            throw new IllegalArgumentException("Invalid radius: must be a valid number");
        }
        
        if (radius < MIN_RADIUS) {
            throw new IllegalArgumentException("Invalid radius: must be non-negative");
        }
        
        if (radius > MAX_RADIUS) {
            throw new IllegalArgumentException("Invalid radius: exceeds maximum allowed value");
        }
        
        // Calculate area: A = π * r²
        double area = PI * radius * radius;
        
        // Check for overflow
        if (Double.isInfinite(area)) {
            throw new ArithmeticException("Area calculation resulted in overflow");
        }
        
        return area;
    }

    public static void main(String[] args) {
        // Test case 1: radius = 5.0
        try {
            double result1 = calculateCircleArea(5.0);
            System.out.println("Test 1 - Radius: 5.0, Area: " + result1);
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }

        // Test case 2: radius = 10.5
        try {
            double result2 = calculateCircleArea(10.5);
            System.out.println("Test 2 - Radius: 10.5, Area: " + result2);
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }

        // Test case 3: radius = 0.0 (edge case)
        try {
            double result3 = calculateCircleArea(0.0);
            System.out.println("Test 3 - Radius: 0.0, Area: " + result3);
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }

        // Test case 4: radius = 1.0
        try {
            double result4 = calculateCircleArea(1.0);
            System.out.println("Test 4 - Radius: 1.0, Area: " + result4);
        } catch (Exception e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }

        // Test case 5: radius = -5.0 (invalid input)
        try {
            double result5 = calculateCircleArea(-5.0);
            System.out.println("Test 5 - Radius: -5.0, Area: " + result5);
        } catch (Exception e) {
            System.err.println("Test 5 failed (expected): " + e.getMessage());
        }
    }
}
