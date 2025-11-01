
import java.util.Arrays;

public class Lse9 {
    /**
     * Retrieves and returns the command line argument at the specified index.
     * 
     * @param args The command line arguments array
     * @param index The index to retrieve (0-based after program name)
     * @return The argument at the specified index, or an error message if invalid
     */
    public static String getArgumentAtIndex(String[] args, int index) {
        // Validate that args is not null to prevent NullPointerException
        if (args == null) {
            return "Error: Arguments array is null";
        }
        
        // Validate index is non-negative to prevent array access violations
        if (index < 0) {
            return "Error: Index cannot be negative";
        }
        
        // Check if index is within bounds to prevent ArrayIndexOutOfBoundsException
        // This prevents buffer over-read vulnerabilities
        if (index >= args.length) {
            return "Error: Index out of bounds";
        }
        
        // Return the argument at the valid index
        return args[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index within bounds
        String[] test1 = {"arg0", "arg1", "arg2", "arg3"};
        System.out.println("Test 1 - Index 1: " + getArgumentAtIndex(test1, 1));
        
        // Test case 2: Valid index at boundary
        String[] test2 = {"first", "second", "third"};
        System.out.println("Test 2 - Index 2: " + getArgumentAtIndex(test2, 2));
        
        // Test case 3: Index out of bounds (should return error)
        String[] test3 = {"only", "two"};
        System.out.println("Test 3 - Index 5: " + getArgumentAtIndex(test3, 5));
        
        // Test case 4: Negative index (should return error)
        String[] test4 = {"some", "args"};
        System.out.println("Test 4 - Index -1: " + getArgumentAtIndex(test4, -1));
        
        // Test case 5: Empty array with index 0 (should return error)
        String[] test5 = {};
        System.out.println("Test 5 - Index 0 on empty: " + getArgumentAtIndex(test5, 0));
        
        // Actual program logic: process command line arguments if provided
        if (args.length > 0) {
            try {
                // Parse the first argument as the index
                // Use Integer.parseInt with validation to prevent NumberFormatException
                int index = Integer.parseInt(args[0]);
                
                // The actual arguments start from index 1 (after the index parameter)
                // Create a subset array excluding the index parameter itself
                String[] actualArgs = Arrays.copyOfRange(args, 1, args.length);
                
                // Retrieve and print the argument at the specified index
                String result = getArgumentAtIndex(actualArgs, index);
                System.out.println(result);
                
            } catch (NumberFormatException e) {
                // Handle invalid number format without exposing internal details
                System.out.println("Error: Invalid index format");
            }
        }
    }
}
