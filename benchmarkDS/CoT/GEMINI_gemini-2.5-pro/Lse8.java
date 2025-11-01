import java.util.Arrays;

public class Lse8 {

    /**
     * Retrieves the command-line argument at the index specified by the first argument.
     *
     * @param args The array of command-line arguments. args[0] is program name,
     *             args[1] should be the integer index to look up.
     * @return The argument at the specified index, or null if an error occurs.
     */
    public static String getValueAtIndex(String[] args) {
        // Security Check 1: Ensure there are enough arguments to proceed.
        // We need at least args[1] (the index) and one more argument to fetch.
        // So, a minimum of 3 arguments (program name, index, value).
        if (args.length < 2) {
            System.err.println("Error: Not enough arguments. Usage: java Lse8 <index> <arg1> <arg2> ...");
            return null;
        }

        int index;
        try {
            // Security Check 2: Safely parse the index from a string to an integer.
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            System.err.println("Error: The first argument (index) must be a valid integer.");
            return null;
        }

        // Security Check 3: Perform bounds checking on the array.
        // The index must be non-negative and less than the total number of arguments.
        if (index < 0 || index >= args.length) {
            System.err.println("Error: Index " + index + " is out of bounds. Valid range is [0, " + (args.length - 1) + "].");
            return null;
        }

        // If all checks pass, return the value at the specified index.
        return args[index];
    }

    public static void main(String[] args) {
        // The main method passes its command-line arguments to the processing function.
        String result = getValueAtIndex(args);
        if (result != null) {
            System.out.println("Value at index " + args[1] + ": " + result);
        }

        // --- Test Cases ---
        // Since this program relies on command-line arguments, you must run it from the terminal.
        // Below are examples of how to run it to test different scenarios.
        //
        // Compile the code first: javac Lse8.java
        //
        // Test Case 1: Successful retrieval
        // Command: java Lse8 3 apple banana orange grape
        // Expected Output: Value at index 3: orange
        //
        // Test Case 2: Index points to the program name itself (index 0)
        // Command: java Lse8 0 test one two
        // Expected Output: Value at index 0: Lse8 
        // Note: In Java, args[0] is the first argument, not the class name. This test is adapted.
        // Command: java Lse8 2 first second third
        // Expected Output: Value at index 2: second
        //
        // Test Case 3: Error - Index out of bounds
        // Command: java Lse8 5 apple banana
        // Expected Output: Error: Index 5 is out of bounds. Valid range is [0, 3].
        //
        // Test Case 4: Error - Invalid index format
        // Command: java Lse8 two apple banana
        // Expected Output: Error: The first argument (index) must be a valid integer.
        //
        // Test Case 5: Error - Not enough arguments
        // Command: java Lse8
        // Expected Output: Error: Not enough arguments. Usage: java Lse8 <index> <arg1> <arg2> ...
    }
}