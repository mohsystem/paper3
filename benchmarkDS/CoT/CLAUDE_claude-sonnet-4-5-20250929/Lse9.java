
public class Lse9 {
    public static String getArgumentAtIndex(String[] args, int index) {
        // Validate input parameters
        if (args == null) {
            return "Error: Arguments array is null";
        }
        
        // Validate index is non-negative
        if (index < 0) {
            return "Error: Index cannot be negative";
        }
        
        // Check if index is within bounds
        if (index >= args.length) {
            return "Error: Index out of bounds";
        }
        
        return args[index];
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        String[] test1 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
        System.out.println(getArgumentAtIndex(test1, 1));
        
        System.out.println("\\nTest Case 2:");
        String[] test2 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
        System.out.println(getArgumentAtIndex(test2, 3));
        
        System.out.println("\\nTest Case 3:");
        String[] test3 = {"arg0", "arg1"};
        System.out.println(getArgumentAtIndex(test3, 0));
        
        System.out.println("\\nTest Case 4:");
        String[] test4 = {"arg0", "arg1", "arg2"};
        System.out.println(getArgumentAtIndex(test4, 5));
        
        System.out.println("\\nTest Case 5:");
        String[] test5 = {"arg0", "arg1", "arg2"};
        System.out.println(getArgumentAtIndex(test5, -1));
        
        // Real command line usage
        if (args.length > 0) {
            try {
                int index = Integer.parseInt(args[0]);
                String result = getArgumentAtIndex(args, index);
                System.out.println("\\nCommand line result: " + result);
            } catch (NumberFormatException e) {
                System.out.println("Error: First argument must be a valid integer");
            }
        }
    }
}
