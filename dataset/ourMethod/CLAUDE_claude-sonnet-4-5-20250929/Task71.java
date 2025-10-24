
import java.nio.charset.StandardCharsets;

public class Task71 {
    /**
     * Converts a string to an integer with validation.
     * @param input The string to convert
     * @return The converted integer
     * @throws IllegalArgumentException if input is invalid
     */
    public static int convertToInteger(String input) {
        // Validate input is not null or empty
        if (input == null || input.trim().isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        
        // Sanitize input - remove leading/trailing whitespace
        String sanitized = input.trim();
        
        // Validate length to prevent potential issues
        if (sanitized.length() > 15) {
            throw new IllegalArgumentException("Input length exceeds maximum allowed");
        }
        
        // Validate characters - only allow digits, optional leading sign, and no other characters
        if (!sanitized.matches("^[+-]?\\\\d+$")) {
            throw new IllegalArgumentException("Input contains invalid characters. Only digits and optional leading +/- allowed");
        }
        
        try {
            // Parse the integer
            long value = Long.parseLong(sanitized);
            
            // Check if value is within Integer range
            if (value < Integer.MIN_VALUE || value > Integer.MAX_VALUE) {
                throw new IllegalArgumentException("Value out of integer range");
            }
            
            return (int) value;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid number format: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "2147483647", "invalid123"};
        
        // Check if command line arguments provided
        if (args.length > 0) {
            for (String arg : args) {
                try {
                    int result = convertToInteger(arg);
                    System.out.println("Input: '" + arg + "' -> Result: " + result);
                } catch (IllegalArgumentException e) {
                    System.err.println("Error for input '" + arg + "': " + e.getMessage());
                }
            }
        } else {
            // Run test cases
            System.out.println("Running test cases:");
            for (String test : testCases) {
                try {
                    int result = convertToInteger(test);
                    System.out.println("Input: '" + test + "' -> Result: " + result);
                } catch (IllegalArgumentException e) {
                    System.err.println("Error for input '" + test + "': " + e.getMessage());
                }
            }
        }
    }
}
