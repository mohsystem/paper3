
public class Lse9 {
    public static String getArgumentAtIndex(String[] args, int index) {
        if (args == null || index < 0 || index >= args.length) {
            return "Error: Invalid index or no argument at the specified index";
        }
        return args[index];
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testArgs1 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
        
        // Test case 1: Get argument at index 1
        System.out.println("Test 1 - Index 1: " + getArgumentAtIndex(testArgs1, 1));
        
        // Test case 2: Get argument at index 3
        System.out.println("Test 2 - Index 3: " + getArgumentAtIndex(testArgs1, 3));
        
        // Test case 3: Get argument at index 0
        System.out.println("Test 3 - Index 0: " + getArgumentAtIndex(testArgs1, 0));
        
        // Test case 4: Invalid index (out of bounds)
        System.out.println("Test 4 - Index 10: " + getArgumentAtIndex(testArgs1, 10));
        
        // Test case 5: Negative index
        System.out.println("Test 5 - Index -1: " + getArgumentAtIndex(testArgs1, -1));
        
        // If actual command line arguments are provided
        if (args.length > 0) {
            try {
                int index = Integer.parseInt(args[0]);
                // Skip the first argument (the index itself) and get from remaining args
                String[] remainingArgs = new String[args.length - 1];
                System.arraycopy(args, 1, remainingArgs, 0, args.length - 1);
                System.out.println("\\nActual argument at index " + index + ": " + 
                                 getArgumentAtIndex(remainingArgs, index));
            } catch (NumberFormatException e) {
                System.out.println("Error: First argument must be a valid integer");
            }
        }
    }
}
