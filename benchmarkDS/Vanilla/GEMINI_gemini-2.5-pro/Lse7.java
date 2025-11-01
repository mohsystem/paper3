public class Lse7 {

    /**
     * Converts the string at index 1 of the args array to an integer 'INDEX',
     * then prints the argument at position 'INDEX'.
     *
     * @param args An array of strings, simulating command-line arguments where
     *             args[0] is the program name, args[1] is the index to look up.
     */
    public static void printArgFromIndex(String[] args) {
        // Check if there are enough arguments to proceed. We need at least 2.
        if (args.length <= 1) {
            System.out.println("Error: Not enough arguments. Please provide an index.");
            return;
        }

        int index;
        try {
            // Convert the second argument (at index 1) to an integer.
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            System.out.println("Error: The second argument must be a valid integer.");
            return;
        }

        // Check if the parsed index is within the valid bounds of the array.
        if (index < 0 || index >= args.length) {
            System.out.println("Error: Index " + index + " is out of bounds for the given arguments.");
            return;
        }

        // Print the argument at the specified index.
        System.out.println(args[index]);
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---

        // Test Case 1: Valid case, should print "banana"
        System.out.println("--- Test Case 1 ---");
        System.out.println("Input: {\"Lse7\", \"3\", \"apple\", \"banana\", \"cherry\"}");
        System.out.print("Output: ");
        String[] test1 = {"Lse7", "3", "apple", "banana", "cherry"};
        printArgFromIndex(test1);
        System.out.println();

        // Test Case 2: Valid case, index 0, should print the program name
        System.out.println("--- Test Case 2 ---");
        System.out.println("Input: {\"Lse7\", \"0\"}");
        System.out.print("Output: ");
        String[] test2 = {"Lse7", "0"};
        printArgFromIndex(test2);
        System.out.println();

        // Test Case 3: Error case, index out of bounds
        System.out.println("--- Test Case 3 ---");
        System.out.println("Input: {\"Lse7\", \"5\", \"a\", \"b\"}");
        System.out.print("Output: ");
        String[] test3 = {"Lse7", "5", "a", "b"};
        printArgFromIndex(test3);
        System.out.println();

        // Test Case 4: Error case, non-integer index
        System.out.println("--- Test Case 4 ---");
        System.out.println("Input: {\"Lse7\", \"two\", \"a\", \"b\"}");
        System.out.print("Output: ");
        String[] test4 = {"Lse7", "two", "a", "b"};
        printArgFromIndex(test4);
        System.out.println();

        // Test Case 5: Error case, not enough arguments
        System.out.println("--- Test Case 5 ---");
        System.out.println("Input: {\"Lse7\"}");
        System.out.print("Output: ");
        String[] test5 = {"Lse7"};
        printArgFromIndex(test5);
        System.out.println();
    }
}