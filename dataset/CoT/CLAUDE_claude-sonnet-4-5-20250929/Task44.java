
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task44 {
    private static final int MAX_INPUT_LENGTH = 100;
    private static final Pattern ALPHANUMERIC_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s]+$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    private static final Pattern NUMERIC_PATTERN = Pattern.compile("^\\\\d+$");
    
    public static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        
        // Trim whitespace
        input = input.trim();
        
        // Check length
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length of " + MAX_INPUT_LENGTH);
        }
        
        // Remove potential SQL injection characters
        input = input.replaceAll("[';\\"\\\\\\\\]", "");
        
        // Remove potential XSS characters
        input = input.replaceAll("[<>]", "");
        
        return input;
    }
    
    public static boolean validateAlphanumeric(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }
        return ALPHANUMERIC_PATTERN.matcher(input).matches();
    }
    
    public static boolean validateEmail(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }
        return EMAIL_PATTERN.matcher(input).matches() && input.length() <= MAX_INPUT_LENGTH;
    }
    
    public static boolean validateNumeric(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }
        return NUMERIC_PATTERN.matcher(input).matches();
    }
    
    public static int safeParseInt(String input, int defaultValue) {
        try {
            if (!validateNumeric(input)) {
                return defaultValue;
            }
            long value = Long.parseLong(input);
            if (value > Integer.MAX_VALUE || value < Integer.MIN_VALUE) {
                return defaultValue;
            }
            return (int) value;
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }
    
    public static String processInput(String input, String inputType) {
        if (input == null || inputType == null) {
            return "Error: Null input provided";
        }
        
        try {
            String sanitized = sanitizeInput(input);
            
            switch (inputType.toLowerCase()) {
                case "alphanumeric":
                    if (validateAlphanumeric(sanitized)) {
                        return "Valid alphanumeric input: " + sanitized;
                    } else {
                        return "Invalid alphanumeric input";
                    }
                    
                case "email":
                    if (validateEmail(sanitized)) {
                        return "Valid email: " + sanitized;
                    } else {
                        return "Invalid email format";
                    }
                    
                case "numeric":
                    if (validateNumeric(sanitized)) {
                        int number = safeParseInt(sanitized, 0);
                        return "Valid number: " + number + ", Doubled: " + (number * 2);
                    } else {
                        return "Invalid numeric input";
                    }
                    
                default:
                    return "Unknown input type";
            }
        } catch (IllegalArgumentException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Input Validation Test Cases ===\\n");
        
        // Test Case 1: Valid alphanumeric input
        System.out.println("Test 1 - Valid Alphanumeric:");
        System.out.println(processInput("Hello World 123", "alphanumeric"));
        System.out.println();
        
        // Test Case 2: Valid email
        System.out.println("Test 2 - Valid Email:");
        System.out.println(processInput("user@example.com", "email"));
        System.out.println();
        
        // Test Case 3: Valid numeric input
        System.out.println("Test 3 - Valid Numeric:");
        System.out.println(processInput("42", "numeric"));
        System.out.println();
        
        // Test Case 4: SQL Injection attempt
        System.out.println("Test 4 - SQL Injection Attempt:");
        System.out.println(processInput("admin' OR '1'='1", "alphanumeric"));
        System.out.println();
        
        // Test Case 5: XSS attempt
        System.out.println("Test 5 - XSS Attempt:");
        System.out.println(processInput("<script>alert('XSS')</script>", "alphanumeric"));
        System.out.println();
    }
}
