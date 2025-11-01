
import java.nio.charset.StandardCharsets;

public class Lse21 {
    public static int processValue(String input) {
        // Input validation: check if input is null or empty
        if (input == null || input.trim().isEmpty()) {
            System.err.println("Error: Input cannot be null or empty");
            return -1;
        }
        
        try {
            // Parse input as integer with validation
            // This prevents injection and ensures type safety
            long value = Long.parseLong(input.trim());
            
            // Check for integer overflow before addition
            // Prevents arithmetic overflow vulnerability
            if (value > Long.MAX_VALUE - 1000) {
                System.err.println("Error: Value would overflow");
                return -1;
            }
            
            if (value < Long.MIN_VALUE + 1000) {
                System.err.println("Error: Value would underflow");
                return -1;
            }
            
            // Safe addition after overflow check
            long result = value + 1000;
            
            // Print result to console using safe output method
            System.out.println(result);
            
            return 0;
        } catch (NumberFormatException e) {
            // Handle parsing errors without exposing internal details
            System.err.println("Error: Invalid number format");
            return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases with input validation
        System.out.println("Test 1: Valid positive number");
        processValue("500");
        
        System.out.println("\\nTest 2: Valid negative number");
        processValue("-200");
        
        System.out.println("\\nTest 3: Zero");
        processValue("0");
        
        System.out.println("\\nTest 4: Large number");
        processValue("999999");
        
        System.out.println("\\nTest 5: Invalid input");
        processValue("not_a_number");
    }
}
