import java.util.Arrays;

public class Lse9 {

    /**
     * Takes command line arguments, parses the first argument as an index,
     * and prints the argument at that index from the remaining arguments.
     *
     * @param args Command line arguments. args[0] is the index, args[1...] are the values.
     * @return The argument at the specified index, or an error message string.
     */
    public static String getArgumentByIndex(String[] args) {
        if (args == null || args.length < 1) {
            return "Error: No index provided. Usage: java Lse9 <index> <arg1> <arg2> ...";
        }

        int index;
        try {
            index = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            return "Error: Invalid index '" + args[0] + "'. Index must be an integer.";
        }

        // The user-provided index N corresponds to args[N].
        // We must check if this index is valid for the args array.
        // A valid index must be >= 0 and < args.length.
        if (index >= 0 && index < args.length) {
            return args[index];
        } else {
            return "Error: Index " + index + " is out of bounds for arguments array of size " + args.length + ".";
        }
    }

    public static void main(String[] args) {
        // This program processes actual command-line arguments.
        // The following comments show 5 test cases and how to run them from the terminal.

        /*
         * To compile and run:
         * javac Lse9.java
         *
         * --- Test Cases ---
         *
         * 1. Normal Case: Get the 2nd argument from the list "apple", "banana", "cherry"
         *    Command: java Lse9 2 apple banana cherry
         *    Expected Output: cherry
         *
         * 2. Edge Case (First Argument): Get the 1st argument
         *    Command: java Lse9 1 apple banana cherry
         *    Expected Output: banana
         *
         * 3. Error Case (Index Out of Bounds): Index is too high
         *    Command: java Lse9 4 apple banana cherry
         *    Expected Output: Error: Index 4 is out of bounds for arguments array of size 4.
         *
         * 4. Error Case (Invalid Index): Index is not a number
         *    Command: java Lse9 foo apple banana
         *    Expected Output: Error: Invalid index 'foo'. Index must be an integer.
         *
         * 5. Error Case (Index points to itself):
         *    Command: java Lse9 0 apple banana
         *    Expected Output: 0
         */

        System.out.println(getArgumentByIndex(args));
    }
}