
import java.nio.charset.StandardCharsets;

public class Lse19 {
    /**
     * Adds 1000 to the provided integer value.
     * 
     * @param value The input integer value
     * @return The input value plus 1000
     * @throws ArithmeticException if addition would cause overflow
     */
    public static int addThousand(int value) {
        // Check for integer overflow before performing addition
        // If value > Integer.MAX_VALUE - 1000, overflow would occur
        if (value > Integer.MAX_VALUE - 1000) {
            throw new ArithmeticException("Integer overflow: result would exceed maximum value");
        }
        // Check for integer underflow (though less likely with adding positive value)
        if (value < Integer.MIN_VALUE + 1000) {
            throw new ArithmeticException("Integer underflow: result would be below minimum value");
        }
        return value + 1000;
    }

    public static void main(String[] args) {
        // Test cases
        int[] testCases = {0, 100, -500, 2147482647, -2147482648};
        
        for (int testValue : testCases) {
            try {
                int result = addThousand(testValue);
                System.out.println("Input: " + testValue + ", Result: " + result);
            } catch (ArithmeticException e) {
                // Generic error message - don't leak internal details\n                System.err.println("Error processing value " + testValue + ": " + e.getMessage());\n            }\n        }\n        \n        // If command line argument provided, process it\n        if (args.length > 0) {\n            try {\n                // Validate input: ensure it's not null or empty
                if (args[0] == null || args[0].trim().isEmpty()) {
                    System.err.println("Error: Invalid input - empty or null value");
                    System.exit(1);
                }
                
                // Validate input length to prevent excessive string processing
                if (args[0].length() > 15) {
                    System.err.println("Error: Invalid input - value too long");
                    System.exit(1);
                }
                
                // Parse input with validation
                int inputValue = Integer.parseInt(args[0].trim());
                int result = addThousand(inputValue);
                System.out.println("Command line input: " + inputValue + ", Result: " + result);
            } catch (NumberFormatException e) {
                // Generic error - don't expose internal details
                System.err.println("Error: Invalid input format");
                System.exit(1);
            } catch (ArithmeticException e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        }
    }
}
