
import java.util.Scanner;

public class Task140 {
    public static Double calculate(double num1, double num2, char operator) {
        switch (operator) {
            case '+':
                return num1 + num2;
            case '-':
                return num1 - num2;
            case '*':
                return num1 * num2;
            case '/':
                if (num2 == 0) {
                    System.out.println("Error: Division by zero");
                    return null;
                }
                return num1 / num2;
            default:
                System.out.println("Error: Invalid operator");
                return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Addition
        System.out.println("Test 1 (10 + 5): " + calculate(10, 5, '+'));
        
        // Test case 2: Subtraction
        System.out.println("Test 2 (10 - 5): " + calculate(10, 5, '-'));
        
        // Test case 3: Multiplication
        System.out.println("Test 3 (10 * 5): " + calculate(10, 5, '*'));
        
        // Test case 4: Division
        System.out.println("Test 4 (10 / 5): " + calculate(10, 5, '/'));
        
        // Test case 5: Division by zero
        System.out.print("Test 5 (10 / 0): ");
        calculate(10, 0, '/');
    }
}
