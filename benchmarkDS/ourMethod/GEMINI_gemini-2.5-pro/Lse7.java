public class Lse7 {

    /**
     * Retrieves an argument from an array at an index specified by another argument.
     *
     * @param args The array of string arguments. Expects args[1] to be the index.
     * @return The argument at the specified index, or an error message if one occurs.
     */
    public static String getArgumentAtIndex(String[] args) {
        // Rule #4: Validate input - ensure there are enough arguments
        if (args == null || args.length < 2) {
            return "Error: Insufficient arguments. At least two are required.";
        }

        int index;
        try {
            // Rule #4: Validate input format - ensure the index is a valid integer
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            return "Error: The second argument must be a valid integer.";
        }

        // Rule #1, #2: Ensure the index is within the buffer's boundaries
        if (index < 0 || index >= args.length) {
            return "Error: Index " + index + " is out of bounds for the arguments array (size: " + args.length + ").";
        }

        return args[index];
    }

    /**
     * Main method to run the program and demonstrate test cases.
     * @param args Command line arguments. If provided, the program will process them.
     */
    public static void main(String[] args) {
        // Process command-line arguments if provided
        if (args.length > 0) {
            System.out.println("--- Processing Command Line Arguments ---");
            System.out.print("Input: [");
            for(int i = 0; i < args.length; i++){
                System.out.print("\"" + args[i] + "\"" + (i == args.length - 1 ? "" : ", "));
            }
            System.out.println("]");
            System.out.println("Output: " + getArgumentAtIndex(args));
            System.out.println("----------------------------------------\n");
        }

        System.out.println("--- Running Built-in Test Cases ---");

        // Test Case 1: Valid index
        String[] test1 = {"arg0", "2", "target_arg", "arg3"};
        System.out.println("\nTest Case 1: Valid index");
        System.out.println("Input: {\"arg0\", \"2\", \"target_arg\", \"arg3\"}");
        System.out.println("Output: " + getArgumentAtIndex(test1));

        // Test Case 2: Index pointing to the first element
        String[] test2 = {"target_arg", "0", "arg2"};
        System.out.println("\nTest Case 2: Index points to first element");
        System.out.println("Input: {\"target_arg\", \"0\", \"arg2\"}");
        System.out.println("Output: " + getArgumentAtIndex(test2));

        // Test Case 3: Index out of bounds (too high)
        String[] test3 = {"arg0", "3"};
        System.out.println("\nTest Case 3: Index out of bounds (high)");
        System.out.println("Input: {\"arg0\", \"3\"}");
        System.out.println("Output: " + getArgumentAtIndex(test3));

        // Test Case 4: Invalid index format (not an integer)
        String[] test4 = {"arg0", "two", "arg2"};
        System.out.println("\nTest Case 4: Invalid index format");
        System.out.println("Input: {\"arg0\", \"two\", \"arg2\"}");
        System.out.println("Output: " + getArgumentAtIndex(test4));

        // Test Case 5: Insufficient arguments
        String[] test5 = {"arg0"};
        System.out.println("\nTest Case 5: Insufficient arguments");
        System.out.println("Input: {\"arg0\"}");
        System.out.println("Output: " + getArgumentAtIndex(test5));
    }
}