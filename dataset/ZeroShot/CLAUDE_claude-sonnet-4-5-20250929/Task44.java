
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task44 {
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final Pattern ALPHANUMERIC_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s.,!?-]+$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    private static final Pattern NUMERIC_PATTERN = Pattern.compile("^-?\\\\d+$");
    
    public static String validateAndSanitizeText(String input) {
        if (input == null) {
            return "Error: Input is null";
        }
        
        if (input.isEmpty()) {
            return "Error: Input is empty";
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + MAX_INPUT_LENGTH;
        }
        
        String trimmed = input.trim();
        
        if (!ALPHANUMERIC_PATTERN.matcher(trimmed).matches()) {
            return "Error: Input contains invalid characters";
        }
        
        String sanitized = trimmed.replaceAll("<", "&lt;")
                                 .replaceAll(">", "&gt;")
                                 .replaceAll("&", "&amp;")
                                 .replaceAll("\\"", "&quot;")
                                 .replaceAll("'", "&#x27;");
        
        return "Valid text: " + sanitized;
    }
    
    public static String validateEmail(String email) {
        if (email == null || email.isEmpty()) {
            return "Error: Email is null or empty";
        }
        
        if (email.length() > 254) {
            return "Error: Email exceeds maximum length";
        }
        
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email format";
        }
        
        return "Valid email: " + email;
    }
    
    public static String validateAndProcessNumber(String input) {
        if (input == null || input.isEmpty()) {
            return "Error: Number input is null or empty";
        }
        
        if (!NUMERIC_PATTERN.matcher(input).matches()) {
            return "Error: Invalid number format";
        }
        
        try {
            long number = Long.parseLong(input);
            long squared = number * number;
            return "Number: " + number + ", Squared: " + squared;
        } catch (NumberFormatException e) {
            return "Error: Number out of range";
        }
    }
    
    public static String processOperation(String operationType, String input) {
        if (operationType == null || input == null) {
            return "Error: Operation type or input is null";
        }
        
        switch (operationType.toLowerCase()) {
            case "text":
                return validateAndSanitizeText(input);
            case "email":
                return validateEmail(input);
            case "number":
                return validateAndProcessNumber(input);
            default:
                return "Error: Unknown operation type";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid Text ===");
        String result1 = processOperation("text", "Hello World 123!");
        System.out.println(result1);
        
        System.out.println("\\n=== Test Case 2: Invalid Text with Special Characters ===");
        String result2 = processOperation("text", "<script>alert('XSS')</script>");
        System.out.println(result2);
        
        System.out.println("\\n=== Test Case 3: Valid Email ===");
        String result3 = processOperation("email", "user@example.com");
        System.out.println(result3);
        
        System.out.println("\\n=== Test Case 4: Valid Number ===");
        String result4 = processOperation("number", "42");
        System.out.println(result4);
        
        System.out.println("\\n=== Test Case 5: Invalid Number ===");
        String result5 = processOperation("number", "abc123");
        System.out.println(result5);
    }
}
