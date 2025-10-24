
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task45 {
    private static final Pattern INTEGER_PATTERN = Pattern.compile("^-?\\\\d+$");
    private static final Pattern DOUBLE_PATTERN = Pattern.compile("^-?\\\\d+\\\\.?\\\\d*$");
    private static final int MAX_INPUT_LENGTH = 1000;
    
    public static String processUserInput(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length of " + MAX_INPUT_LENGTH);
        }
        
        String trimmedInput = input.trim();
        
        if (trimmedInput.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be empty");
        }
        
        return trimmedInput;
    }
    
    public static int parseInteger(String input) {
        String processed = processUserInput(input);
        
        if (!INTEGER_PATTERN.matcher(processed).matches()) {
            throw new NumberFormatException("Invalid integer format: " + processed);
        }
        
        try {
            return Integer.parseInt(processed);
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Integer out of range: " + processed);
        }
    }
    
    public static double parseDouble(String input) {
        String processed = processUserInput(input);
        
        if (!DOUBLE_PATTERN.matcher(processed).matches()) {
            throw new NumberFormatException("Invalid double format: " + processed);
        }
        
        try {
            double result = Double.parseDouble(processed);
            if (Double.isInfinite(result) || Double.isNaN(result)) {
                throw new NumberFormatException("Double out of range or invalid: " + processed);
            }
            return result;
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Invalid double value: " + processed);
        }
    }
    
    public static int safeDivide(int numerator, int denominator) {
        if (denominator == 0) {
            throw new ArithmeticException("Division by zero is not allowed");
        }
        
        if (numerator == Integer.MIN_VALUE && denominator == -1) {
            throw new ArithmeticException("Integer overflow in division");
        }
        
        return numerator / denominator;
    }
    
    public static int safeAdd(int a, int b) {
        long result = (long) a + (long) b;
        if (result > Integer.MAX_VALUE || result < Integer.MIN_VALUE) {
            throw new ArithmeticException("Integer overflow in addition");
        }
        return (int) result;
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid integer parsing ===");
        try {
            int result = parseInteger("42");
            System.out.println("Parsed integer: " + result);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 2: Invalid integer (non-numeric) ===");
        try {
            int result = parseInteger("abc");
            System.out.println("Parsed integer: " + result);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 3: Division by zero ===");
        try {
            int result = safeDivide(10, 0);
            System.out.println("Division result: " + result);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 4: Valid double parsing ===");
        try {
            double result = parseDouble("3.14159");
            System.out.println("Parsed double: " + result);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 5: Integer overflow protection ===");
        try {
            int result = safeAdd(Integer.MAX_VALUE, 1);
            System.out.println("Addition result: " + result);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
