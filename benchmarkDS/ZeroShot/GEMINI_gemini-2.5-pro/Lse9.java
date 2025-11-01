public class Lse9 {

    /**
     * Retrieves a command-line argument by a 1-based index provided as the first argument.
     *
     * @param args The command-line arguments array. args[0] is the index, args[1...] are the data.
     * @return The argument at the specified index.
     * @throws IllegalArgumentException if arguments are invalid.
     */
    public static String getArgumentByIndex(String[] args) {
        // 1. Security Check: Ensure enough arguments are provided (index + at least one data arg)
        if (args.length < 2) {
            throw new IllegalArgumentException("Usage: java Lse9 <index> <arg1> [arg2] ...");
        }

        int index;
        try {
            // 2. Security Check: Parse the index, handle non-integer input
            index = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Error: Index must be a valid integer.");
        }

        // 3. Security Check: Ensure the index is positive (1-based)
        if (index < 1) {
            throw new IllegalArgumentException("Error: Index must be a positive integer (1-based).");
        }
        
        // The number of data arguments available is the total length minus the index argument.
        int dataArgCount = args.length - 1;

        // 4. Security Check: Ensure the index is within the bounds of the available data arguments.
        if (index > dataArgCount) {
            throw new IllegalArgumentException("Error: Index " + index + " is out of bounds. Only " + dataArgCount + " data arguments available.");
        }

        // The user's 1-based index corresponds to the array index 'index' in `args`
        // because args[0] is the index itself, and we want to access the subsequent elements.
        return args[index];
    }

    public static void main(String[] args) {
        // Note: These test cases are for demonstration.
        // You must run this program from the command line to provide arguments.

        /*
        To compile and run from the command line:
        javac Lse9.java
        
        Test Cases:
        1. Valid case: Get the 2nd data argument
           Command: java Lse9 2 apple banana orange
           Expected Output: banana

        2. Valid case: Get the 1st data argument
           Command: java Lse9 1 apple banana orange
           Expected Output: apple

        3. Error case: Index out of bounds
           Command: java Lse9 4 apple banana orange
           Expected Output: Error: Index 4 is out of bounds. Only 3 data arguments available.

        4. Error case: Non-integer index
           Command: java Lse9 two apple banana
           Expected Output: Error: Index must be a valid integer.

        5. Error case: Not enough arguments
           Command: java Lse9 1
           Expected Output: Usage: java Lse9 <index> <arg1> [arg2] ...
        */
        
        try {
            String argument = getArgumentByIndex(args);
            System.out.println(argument);
        } catch (IllegalArgumentException e) {
            System.err.println(e.getMessage());
        }
    }
}