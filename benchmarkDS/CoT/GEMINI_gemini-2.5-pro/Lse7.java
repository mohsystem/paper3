public class Lse7 {

    /**
     * Converts a string to an integer.
     * In Java, this is typically handled by Integer.parseInt, which throws
     * NumberFormatException on invalid input. We'll handle that exception.
     * @param s The string to convert.
     * @return The integer value.
     * @throws NumberFormatException if the string is not a valid integer.
     */
    public static int convertStringToInt(String s) throws NumberFormatException {
        return Integer.parseInt(s);
    }

    /**
     * Prints the command line argument at the index specified by the second argument.
     * @param args The array of command line arguments.
     */
    public static void printArgumentAtIndex(String[] args) {
        // 1. Check if there are enough arguments (program name + index)
        if (args.length < 2) {
            System.out.println("Error: Not enough arguments. Usage: <program> <index> [arg1] [arg2] ...");
            return;
        }

        try {
            // 2. Convert the second argument (at index 1) to an integer
            String indexString = args[1];
            int index = convertStringToInt(indexString);

            // 3. Check if the calculated index is within the bounds of the args array
            if (index >= 0 && index < args.length) {
                // 4. Print the argument at the specified index
                System.out.println(args[index]);
            } else {
                System.out.println("Error: Index " + index + " is out of bounds. Valid range is 0 to " + (args.length - 1) + ".");
            }
        } catch (NumberFormatException e) {
            // Handle cases where the second argument is not a valid integer
            System.out.println("Error: The second argument '" + args[1] + "' must be a valid integer.");
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running with actual command line arguments ---");
        // To test this part, run from command line:
        // java Lse7.java 3 apple banana orange grape
        // Expected output: grape
        if (args.length > 0) {
            printArgumentAtIndex(args);
        } else {
            System.out.println("No command line arguments provided for live test.");
        }
        
        System.out.println("\n--- Running with 5 test cases ---");

        // Test Case 1: Valid input
        System.out.print("Test 1: ");
        String[] test1 = {"program", "3", "zero", "one", "two", "three"};
        printArgumentAtIndex(test1); // Expected: one

        // Test Case 2: Index out of bounds (too high)
        System.out.print("Test 2: ");
        String[] test2 = {"program", "5", "a", "b", "c"};
        printArgumentAtIndex(test2); // Expected: Error: Index 5 is out of bounds.

        // Test Case 3: Index is not a number
        System.out.print("Test 3: ");
        String[] test3 = {"program", "two", "a", "b", "c"};
        printArgumentAtIndex(test3); // Expected: Error: The second argument 'two' must be a valid integer.

        // Test Case 4: Not enough arguments
        System.out.print("Test 4: ");
        String[] test4 = {"program"};
        printArgumentAtIndex(test4); // Expected: Error: Not enough arguments.

        // Test Case 5: Index is negative
        System.out.print("Test 5: ");
        String[] test5 = {"program", "-1", "a", "b"};
        printArgumentAtIndex(test5); // Expected: Error: Index -1 is out of bounds.
    }
}