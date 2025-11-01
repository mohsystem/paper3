public class Lse8 {

    /**
     * Retrieves the value from the arguments array at the index specified by args[1].
     *
     * @param args The array of command-line arguments.
     * @return The string at the specified index or an error message.
     */
    public static String getValueAtIndex(String[] args) {
        if (args.length < 2) {
            return "Error: Not enough arguments. Please provide an index.";
        }

        int index;
        try {
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            return "Error: The index provided in the second argument must be an integer.";
        }

        if (index < 0 || index >= args.length) {
            return "Error: Index " + index + " is out of bounds for the given arguments.";
        }

        return args[index];
    }

    public static void main(String[] args) {
        // Example with actual command line arguments
        System.out.println("--- Processing Actual Command Line Arguments ---");
        System.out.println("Arguments received: " + java.util.Arrays.toString(args));
        String result = getValueAtIndex(args);
        System.out.println("Result: " + result);
        System.out.println("\n--- Running 5 Hardcoded Test Cases ---");

        // Test Case 1: Valid case
        String[] test1 = {"program", "3", "apple", "banana", "cherry"};
        System.out.println("Test 1: " + java.util.Arrays.toString(test1));
        System.out.println("Output: " + getValueAtIndex(test1)); // Expected: cherry

        // Test Case 2: Index out of bounds
        String[] test2 = {"program", "5", "one", "two"};
        System.out.println("\nTest 2: " + java.util.Arrays.toString(test2));
        System.out.println("Output: " + getValueAtIndex(test2)); // Expected: Error

        // Test Case 3: Index is not a number
        String[] test3 = {"program", "foo", "bar"};
        System.out.println("\nTest 3: " + java.util.Arrays.toString(test3));
        System.out.println("Output: " + getValueAtIndex(test3)); // Expected: Error

        // Test Case 4: Not enough arguments
        String[] test4 = {"program"};
        System.out.println("\nTest 4: " + java.util.Arrays.toString(test4));
        System.out.println("Output: " + getValueAtIndex(test4)); // Expected: Error

        // Test Case 5: Index points to itself
        String[] test5 = {"program", "1", "arg2", "arg3"};
        System.out.println("\nTest 5: " + java.util.Arrays.toString(test5));
        System.out.println("Output: " + getValueAtIndex(test5)); // Expected: 1
    }
}