public class Lse9 {

    /**
     * Retrieves a command-line argument by an index specified in the first argument.
     *
     * @param args The command-line arguments array. args[0] is expected to be the index.
     * @return The argument at the specified index, or an error message.
     */
    public static String getArgumentAtIndex(String[] args) {
        if (args == null || args.length < 1) {
            return "Error: No index provided.";
        }

        int targetIndex;
        try {
            targetIndex = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            return "Error: The first argument must be a valid integer index.";
        }

        // The index argument itself is at args[0].
        // Therefore, any valid target index must be greater than 0.
        if (targetIndex <= 0) {
            return "Error: Index must be a positive integer.";
        }

        if (targetIndex >= args.length) {
            return "Error: Index " + targetIndex + " is out of bounds. There are only " + args.length + " user-provided arguments.";
        }

        return args[targetIndex];
    }

    public static void main(String[] args) {
        // This main method demonstrates the logic.
        // To run with actual command-line arguments, compile and execute:
        // javac Lse9.java
        // java Lse9 2 zero one two three  (should print "two")
        // java Lse9 1 one two three      (should print "one")
        // java Lse9 4 zero one             (should print error)
        // java Lse9 -1 zero one            (should print error)
        // java Lse9 0 zero one             (should print error)
        // java Lse9 abc zero one           (should print error)

        System.out.println("--- Running with actual command line arguments ---");
        System.out.println("Output: " + getArgumentAtIndex(args));
        System.out.println("------------------------------------------------\n");

        System.out.println("--- Running with simulated test cases ---");

        // Test Case 1: Valid index
        String[] test1 = {"2", "apple", "banana", "cherry"};
        System.out.println("Test Case 1: java Lse9 2 apple banana cherry");
        System.out.println("Output: " + getArgumentAtIndex(test1)); // Expected: banana
        System.out.println();

        // Test Case 2: Index out of bounds (too high)
        String[] test2 = {"3", "dog", "cat"};
        System.out.println("Test Case 2: java Lse9 3 dog cat");
        System.out.println("Output: " + getArgumentAtIndex(test2)); // Expected: Error
        System.out.println();

        // Test Case 3: Invalid index (not a number)
        String[] test3 = {"one", "two", "three"};
        System.out.println("Test Case 3: java Lse9 one two three");
        System.out.println("Output: " + getArgumentAtIndex(test3)); // Expected: Error
        System.out.println();
        
        // Test Case 4: Invalid index (zero)
        String[] test4 = {"0", "a", "b"};
        System.out.println("Test Case 4: java Lse9 0 a b");
        System.out.println("Output: " + getArgumentAtIndex(test4)); // Expected: Error
        System.out.println();

        // Test Case 5: Invalid index (negative)
        String[] test5 = {"-1", "a", "b"};
        System.out.println("Test Case 5: java Lse9 -1 a b");
        System.out.println("Output: " + getArgumentAtIndex(test5)); // Expected: Error
        System.out.println();
    }
}