
import java.util.*;
import java.util.regex.Pattern;

public class Task45 {
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final Pattern ALPHANUMERIC_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s]+$");
    
    // Secure calculator with input validation
    public static double calculator(String operation, double num1, double num2) {
        if (operation == null || operation.trim().isEmpty()) {
            throw new IllegalArgumentException("Operation cannot be null or empty");
        }
        
        if (Double.isNaN(num1) || Double.isNaN(num2) || 
            Double.isInfinite(num1) || Double.isInfinite(num2)) {
            throw new IllegalArgumentException("Invalid number input");
        }
        
        switch (operation.toLowerCase().trim()) {
            case "add":
                return num1 + num2;
            case "subtract":
                return num1 - num2;
            case "multiply":
                return num1 * num2;
            case "divide":
                if (num2 == 0) {
                    throw new ArithmeticException("Division by zero is not allowed");
                }
                return num1 / num2;
            default:
                throw new IllegalArgumentException("Invalid operation: " + operation);
        }
    }
    
    // Secure string validator with length and character checks
    public static boolean validateInput(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum allowed length");
        }
        
        return ALPHANUMERIC_PATTERN.matcher(input).matches();
    }
    
    // Secure array processor with bounds checking
    public static int processArray(int[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        
        if (arr.length == 0) {
            throw new IllegalArgumentException("Array cannot be empty");
        }
        
        if (index < 0 || index >= arr.length) {
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array length " + arr.length);
        }
        
        return arr[index] * 2;
    }
    
    // Secure string parser with error handling
    public static int parseSecureInteger(String input) {
        if (input == null || input.trim().isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        
        String trimmed = input.trim();
        if (trimmed.length() > 10) {
            throw new IllegalArgumentException("Input string too long for integer parsing");
        }
        
        try {
            return Integer.parseInt(trimmed);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid integer format: " + trimmed, e);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Calculator Operations ===");
        try {
            System.out.println("Add 10 + 5 = " + calculator("add", 10, 5));
            System.out.println("Divide 20 / 4 = " + calculator("divide", 20, 4));
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 2: Division by Zero ===");
        try {
            calculator("divide", 10, 0);
        } catch (ArithmeticException e) {
            System.err.println("Caught expected error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 3: Input Validation ===");
        try {
            System.out.println("Valid input 'Hello123': " + validateInput("Hello123"));
            System.out.println("Invalid input 'Hello@123': " + validateInput("Hello@123"));
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 4: Array Processing ===");
        try {
            int[] testArray = {1, 2, 3, 4, 5};
            System.out.println("Process array[2] = " + processArray(testArray, 2));
            System.out.println("Process array[10] = " + processArray(testArray, 10));
        } catch (IndexOutOfBoundsException e) {
            System.err.println("Caught expected error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 5: String Parsing ===");
        try {
            System.out.println("Parse '123' = " + parseSecureInteger("123"));
            System.out.println("Parse 'abc' = " + parseSecureInteger("abc"));
        } catch (IllegalArgumentException e) {
            System.err.println("Caught expected error: " + e.getMessage());
        }
    }
}
