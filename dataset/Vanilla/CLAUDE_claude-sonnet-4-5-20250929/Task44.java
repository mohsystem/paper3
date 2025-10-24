
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task44 {
    private static final int MAX_INPUT_LENGTH = 100;
    private static final Pattern ALPHANUMERIC_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s]+$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    private static final Pattern NUMERIC_PATTERN = Pattern.compile("^-?\\\\d+$");

    public static String validateAndSanitizeInput(String input) {
        if (input == null || input.isEmpty()) {
            return "Error: Input cannot be null or empty";
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + MAX_INPUT_LENGTH;
        }
        
        // Remove potentially dangerous characters
        String sanitized = input.replaceAll("[<>\\"';(){}\\\\[\\\\]\\\\\\\\]", "");
        return sanitized.trim();
    }

    public static String validateAlphanumeric(String input) {
        String sanitized = validateAndSanitizeInput(input);
        if (sanitized.startsWith("Error:")) {
            return sanitized;
        }
        
        if (!ALPHANUMERIC_PATTERN.matcher(sanitized).matches()) {
            return "Error: Input must contain only alphanumeric characters and spaces";
        }
        
        return "Valid alphanumeric: " + sanitized;
    }

    public static String validateEmail(String input) {
        String sanitized = validateAndSanitizeInput(input);
        if (sanitized.startsWith("Error:")) {
            return sanitized;
        }
        
        if (!EMAIL_PATTERN.matcher(sanitized).matches()) {
            return "Error: Invalid email format";
        }
        
        return "Valid email: " + sanitized;
    }

    public static String validateNumeric(String input) {
        String sanitized = validateAndSanitizeInput(input);
        if (sanitized.startsWith("Error:")) {
            return sanitized;
        }
        
        if (!NUMERIC_PATTERN.matcher(sanitized).matches()) {
            return "Error: Input must be a valid number";
        }
        
        return "Valid number: " + sanitized;
    }

    public static String processInput(String input, String type) {
        if (type == null || type.isEmpty()) {
            return "Error: Validation type not specified";
        }
        
        switch (type.toLowerCase()) {
            case "alphanumeric":
                return validateAlphanumeric(input);
            case "email":
                return validateEmail(input);
            case "numeric":
                return validateNumeric(input);
            default:
                return "Error: Unknown validation type";
        }
    }

    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid Alphanumeric ===");
        System.out.println(processInput("Hello World 123", "alphanumeric"));
        
        System.out.println("\\n=== Test Case 2: Valid Email ===");
        System.out.println(processInput("user@example.com", "email"));
        
        System.out.println("\\n=== Test Case 3: Valid Numeric ===");
        System.out.println(processInput("12345", "numeric"));
        
        System.out.println("\\n=== Test Case 4: Invalid Input with Special Characters ===");
        System.out.println(processInput("<script>alert('xss')</script>", "alphanumeric"));
        
        System.out.println("\\n=== Test Case 5: Input Too Long ===");
        String longInput = "a".repeat(150);
        System.out.println(processInput(longInput, "alphanumeric"));
    }
}
