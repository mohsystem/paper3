
public class Task140 {
    public static double calculate(double num1, double num2, String operator) {
        // Validate operator input (whitelist approach)
        if (operator == null || operator.length() != 1) {
            throw new IllegalArgumentException("Invalid operator");
        }
        
        char op = operator.charAt(0);
        
        switch (op) {
            case '+':
                return num1 + num2;
            case '-':
                return num1 - num2;
            case '*':
                return num1 * num2;
            case '/':
                // Secure division with zero-check
                if (num2 == 0) {
                    throw new ArithmeticException("Division by zero is not allowed");
                }
                return num1 / num2;
            default:
                throw new IllegalArgumentException("Invalid operator. Use +, -, *, or /");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Addition
        try {
            System.out.println("Test 1: 10 + 5 = " + calculate(10, 5, "+"));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 2: Subtraction
        try {
            System.out.println("Test 2: 20 - 8 = " + calculate(20, 8, "-"));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 3: Multiplication
        try {
            System.out.println("Test 3: 7 * 6 = " + calculate(7, 6, "*"));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 4: Division
        try {
            System.out.println("Test 4: 15 / 3 = " + calculate(15, 3, "/"));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 5: Division by zero (error handling)
        try {
            System.out.println("Test 5: 10 / 0 = " + calculate(10, 0, "/"));
        } catch (Exception e) {
            System.out.println("Test 5 Error: " + e.getMessage());
        }
    }
}
