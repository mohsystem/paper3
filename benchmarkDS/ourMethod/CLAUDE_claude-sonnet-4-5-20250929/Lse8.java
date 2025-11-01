
public class Lse8 {
    /**
     * Returns the command line argument at the index specified by argv[1].
     * 
     * @param args Command line arguments array
     * @return The argument at the specified index, or null if invalid
     */
    public static String getArgumentAtIndex(String[] args) {
        // Check if we have at least 2 arguments (program name equivalent and index)
        if (args == null || args.length < 2) {
            return null;
        }
        
        try {
            // Parse the index from the second argument (argv[1])
            // This could throw NumberFormatException if not a valid integer
            int index = Integer.parseInt(args[1]);
            
            // Validate index is non-negative to prevent negative array access
            if (index < 0) {
                return null;
            }
            
            // Check bounds to prevent ArrayIndexOutOfBoundsException
            // Index must be less than array length
            if (index >= args.length) {
                return null;
            }
            
            // Return the value at the validated index
            return args[index];
            
        } catch (NumberFormatException e) {
            // argv[1] is not a valid integer
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index within bounds
        String[] test1 = {"program", "2", "hello", "world"};
        System.out.println("Test 1: " + getArgumentAtIndex(test1)); // Expected: "hello"
        
        // Test case 2: Index out of bounds
        String[] test2 = {"program", "5", "hello"};
        System.out.println("Test 2: " + getArgumentAtIndex(test2)); // Expected: null
        
        // Test case 3: Negative index
        String[] test3 = {"program", "-1", "hello"};
        System.out.println("Test 3: " + getArgumentAtIndex(test3)); // Expected: null
        
        // Test case 4: Invalid index format
        String[] test4 = {"program", "abc", "hello"};
        System.out.println("Test 4: " + getArgumentAtIndex(test4)); // Expected: null
        
        // Test case 5: Insufficient arguments
        String[] test5 = {"program"};
        System.out.println("Test 5: " + getArgumentAtIndex(test5)); // Expected: null
    }
}
