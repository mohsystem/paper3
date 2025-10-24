
import java.util.Scanner;

public class Task139 {
    public static double calculateArea(double radius) {
        return Math.PI * radius * radius;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Radius: 5.0");
        System.out.println("Area: " + calculateArea(5.0));
        System.out.println();
        
        System.out.println("Test Case 2 - Radius: 10.5");
        System.out.println("Area: " + calculateArea(10.5));
        System.out.println();
        
        System.out.println("Test Case 3 - Radius: 1.0");
        System.out.println("Area: " + calculateArea(1.0));
        System.out.println();
        
        System.out.println("Test Case 4 - Radius: 7.25");
        System.out.println("Area: " + calculateArea(7.25));
        System.out.println();
        
        System.out.println("Test Case 5 - Radius: 15.8");
        System.out.println("Area: " + calculateArea(15.8));
    }
}
