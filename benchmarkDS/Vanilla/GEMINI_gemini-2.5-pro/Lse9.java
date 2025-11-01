public class Lse9 {

    /**
     * Prints a command-line argument based on a given index.
     * The index is provided as the first argument, and it refers to the
     * subsequent arguments. For example, an index of 1 refers to the first
     * argument after the index itself.
     * @param args The array of command-line arguments.
     */
    public static void printArgByIndex(String[] args) {
        // In Java, args[] does not include the program name.
        // We need at least two arguments: an index and a value.
        if (args.length < 1) {
            System.out.println("Usage: java Lse9 <index> <arg1> <arg2> ...");
            return;
        }

        try {
            // The first argument (args[0]) is the user-provided index.
            int userIndex = Integer.parseInt(args[0]);

            if (userIndex <= 0) {
                 System.out.println("Error: Index must be a positive integer.");
                 return;
            }

            // The target argument is at args[userIndex] because args[0] is the index itself.
            if (args.length > userIndex) {
                System.out.println(args[userIndex]);
            } else {
                System.out.println("Error: Index " + userIndex + " is out of bounds for the provided arguments.");
            }
        } catch (NumberFormatException e) {
            System.out.println("Error: The first argument must be a valid integer index.");
        }
    }

    public static void main(String[] args) {
        /*
         * To run the test cases, compile the code and execute from the command line:
         *
         * javac Lse9.java
         *
         * Test Case 1: Get the 1st argument after the index
         * Command: java Lse9 1 apple banana cherry
         * Expected Output: apple
         *
         * Test Case 2: Get the 3rd argument after the index
         * Command: java Lse9 3 apple banana cherry
         * Expected Output: cherry
         *
         * Test Case 3: Index out of bounds
         * Command: java Lse9 4 apple banana cherry
         * Expected Output: Error: Index 4 is out of bounds for the provided arguments.
         *
         * Test Case 4: Not enough arguments (no argument to select)
         * Command: java Lse9 1
         * Expected Output: Error: Index 1 is out of bounds for the provided arguments.
         *
         * Test Case 5: Invalid index (not an integer)
         * Command: java Lse9 hello world
         * Expected Output: Error: The first argument must be a valid integer index.
         */
        printArgByIndex(args);
    }
}