
import java.util.Arrays;

public class Task71 {
    /**
     * Safely converts a string to an integer with proper error handling
     * @param input The string to convert
     * @return The converted integer, or null if conversion fails
     */
    public static Integer convertToInteger(String input) {
        // Input validation
        if (input == null || input.trim().isEmpty()) {
            System.err.println("Error: Input is null or empty");
            return null;
        }
        
        try {
            // Trim whitespace and attempt conversion
            String trimmedInput = input.trim();
            
            // Additional validation for potential overflow
            long longValue = Long.parseLong(trimmedInput);
            if (longValue > Integer.MAX_VALUE || longValue < Integer.MIN_VALUE) {
                System.err.println("Error: Value out of integer range: " + trimmedInput);
                return null;
            }
            
            return Integer.parseInt(trimmedInput);
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid integer format: '" + input + "'");
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Security: Validate command line arguments exist
        if (args.length == 0) {
            System.err.println("Error: No command line arguments provided");
            System.out.println("Usage: java Task71 <integer_string>");
            return;
        }
        
        System.out.println("=== Command Line Argument Conversion ===");
        for (int i = 0; i < args.length; i++) {
            System.out.println("\\nArgument " + (i + 1) + ": " + args[i]);
            Integer result = convertToInteger(args[i]);
            if (result != null) {
                System.out.println("Converted successfully: " + result);
            }
        }
        
        // Test cases
        System.out.println("\\n=== Running Test Cases ===");
        String[] testCases = {
            "42",                    // Valid positive integer
            "-123",                  // Valid negative integer
            "0",                     // Zero
            "2147483647",           // MAX_VALUE
            "invalid123",           // Invalid format
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\\nTest Case " + (i + 1) + ": \\"" + testCases[i] + "\\"");
            Integer result = convertToInteger(testCases[i]);
            if (result != null) {
                System.out.println("Result: " + result);
            }
        }
    }
}
