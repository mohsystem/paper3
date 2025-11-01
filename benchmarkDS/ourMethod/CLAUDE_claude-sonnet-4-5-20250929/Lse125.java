
import java.util.Arrays;

public class Lse125 {
    /**
     * Trims trailing whitespace from a string and returns the result.
     * This approach avoids potential security issues by:
     * - Not using regex or complex parsing that could be exploited
     * - Using simple array operations with bounds checking
     * - Handling null input safely
     */
    public static String trimTrailingWhitespace(String input) {
        // Validate input - fail safely on null
        if (input == null) {
            return "";
        }
        
        // Handle empty string
        if (input.isEmpty()) {
            return input;
        }
        
        // Find the last non-whitespace character
        // Start from the end and work backwards
        int lastNonWhitespace = input.length() - 1;
        while (lastNonWhitespace >= 0 && Character.isWhitespace(input.charAt(lastNonWhitespace))) {
            lastNonWhitespace--;
        }
        
        // If all characters are whitespace, return empty string
        if (lastNonWhitespace < 0) {
            return "";
        }
        
        // Return substring up to and including the last non-whitespace character
        // substring is bounds-safe in Java
        return input.substring(0, lastNonWhitespace + 1);
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "hello world   ",
            "no trailing",
            "   leading and trailing   ",
            "   ",
            ""
        };
        
        System.out.println("Test cases:");
        for (int i = 0; i < testCases.length; i++) {
            String result = trimTrailingWhitespace(testCases[i]);
            System.out.println("Test " + (i + 1) + ": [" + result + "]");
        }
        
        // Process command line arguments if provided
        // Validate that arguments exist before accessing
        if (args.length > 0) {
            System.out.println("\\nCommand line argument processing:");
            // Only process the first argument to limit resource usage
            String input = args[0];
            
            // Validate input length to prevent resource exhaustion
            final int MAX_INPUT_LENGTH = 10000;
            if (input.length() > MAX_INPUT_LENGTH) {
                System.err.println("Error: Input exceeds maximum allowed length");
                System.exit(1);
            }
            
            String trimmed = trimTrailingWhitespace(input);
            System.out.println("[" + trimmed + "]");
        }
    }
}
