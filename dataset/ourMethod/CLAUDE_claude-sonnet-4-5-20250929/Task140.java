
import java.util.Scanner;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.HashSet;

public class Task140 {
    private static final Set<String> VALID_OPERATORS = new HashSet<>();
    
    static {
        VALID_OPERATORS.add("+");
        VALID_OPERATORS.add("-");
        VALID_OPERATORS.add("*");
        VALID_OPERATORS.add("/");
    }
    
    public static String calculate(double num1, double num2, String operator) {
        if (operator == null || operator.isEmpty()) {
            return "Error: Operator cannot be null or empty";
        }
        
        if (!VALID_OPERATORS.contains(operator)) {
            return "Error: Invalid operator. Use +, -, *, or /";
        }
        
        if (Double.isNaN(num1) || Double.isInfinite(num1)) {
            return "Error: First number is invalid";
        }
        
        if (Double.isNaN(num2) || Double.isInfinite(num2)) {
            return "Error: Second number is invalid";
        }
        
        double result;
        switch (operator) {
            case "+":
                result = num1 + num2;
                break;
            case "-":
                result = num1 - num2;
                break;
            case "*":
                result = num1 * num2;
                break;
            case "/":
                if (num2 == 0.0) {
                    return "Error: Division by zero";
                }
                result = num1 / num2;
                break;
            default:
                return "Error: Invalid operator";
        }
        
        if (Double.isNaN(result) || Double.isInfinite(result)) {
            return "Error: Result is invalid";
        }
        
        return String.format("%.6f", result);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: 10 + 5 = " + calculate(10, 5, "+"));
        System.out.println("Test Case 2: 10 - 5 = " + calculate(10, 5, "-"));
        System.out.println("Test Case 3: 10 * 5 = " + calculate(10, 5, "*"));
        System.out.println("Test Case 4: 10 / 5 = " + calculate(10, 5, "/"));
        System.out.println("Test Case 5: 10 / 0 = " + calculate(10, 0, "/"));
    }
}
