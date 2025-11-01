
public class Lse8 {
    public static String getArgumentAtIndex(String[] args) {
        // Security check: Validate arguments array
        if (args == null || args.length < 2) {
            return "Error: Insufficient arguments. Need at least 2 arguments.";
        }
        
        try {
            // Security check: Safely parse the index
            int index = Integer.parseInt(args[1]);
            
            // Security check: Validate index bounds
            if (index < 0 || index >= args.length) {
                return "Error: Index out of bounds. Valid range: 0 to " + (args.length - 1);
            }
            
            // Return the value at the specified index
            return args[index];
            
        } catch (NumberFormatException e) {
            return "Error: Invalid index format. Index must be an integer.";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal case
        String[] test1 = {"program", "2", "hello", "world"};
        System.out.println("Test 1: " + getArgumentAtIndex(test1));
        
        // Test case 2: Index out of bounds
        String[] test2 = {"program", "10", "hello"};
        System.out.println("Test 2: " + getArgumentAtIndex(test2));
        
        // Test case 3: Invalid index format
        String[] test3 = {"program", "abc", "hello"};
        System.out.println("Test 3: " + getArgumentAtIndex(test3));
        
        // Test case 4: Insufficient arguments
        String[] test4 = {"program"};
        System.out.println("Test 4: " + getArgumentAtIndex(test4));
        
        // Test case 5: Negative index
        String[] test5 = {"program", "-1", "hello"};
        System.out.println("Test 5: " + getArgumentAtIndex(test5));
    }
}
