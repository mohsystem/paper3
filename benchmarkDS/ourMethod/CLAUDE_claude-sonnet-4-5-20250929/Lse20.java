
import java.nio.charset.StandardCharsets;

public class Lse20 {
    private static final int OFFSET = 1000;
    private static final int MAX_INPUT_VALUE = Integer.MAX_VALUE - OFFSET; // Prevent overflow
    private static final int MIN_INPUT_VALUE = Integer.MIN_VALUE - OFFSET; // Prevent underflow
    
    public static int addOffset(String input) {
        // Input validation: check for null or empty input
        if (input == null || input.trim().isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        
        // Sanitize input: remove leading/trailing whitespace
        String sanitized = input.trim();
        
        try {
            // Parse input as integer with validation
            int value = Integer.parseInt(sanitized);
            
            // Integer overflow/underflow check before addition
            if (value > MAX_INPUT_VALUE) {
                throw new ArithmeticException("Input value too large, would cause overflow");
            }
            if (value < MIN_INPUT_VALUE) {
                throw new ArithmeticException("Input value too small, would cause underflow");
            }
            
            // Safe addition after validation
            return value + OFFSET;
        } catch (NumberFormatException e) {
            // Avoid leaking internal details in error message
            throw new IllegalArgumentException("Invalid numeric input");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal positive value
        try {
            String input1 = "500";
            int result1 = addOffset(input1);
            System.out.println("Input: " + input1 + ", Result: " + result1);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 2: Negative value
        try {
            String input2 = "-200";
            int result2 = addOffset(input2);
            System.out.println("Input: " + input2 + ", Result: " + result2);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 3: Zero
        try {
            String input3 = "0";
            int result3 = addOffset(input3);
            System.out.println("Input: " + input3 + ", Result: " + result3);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 4: Large value near max integer (should prevent overflow)
        try {
            String input4 = "2147482647";
            int result4 = addOffset(input4);
            System.out.println("Input: " + input4 + ", Result: " + result4);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        // Test case 5: Invalid input
        try {
            String input5 = "invalid";
            int result5 = addOffset(input5);
            System.out.println("Input: " + input5 + ", Result: " + result5);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
