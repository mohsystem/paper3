import java.util.Arrays;

public class Lse8 {

    /**
     * Retrieves the argument at the index specified by args[1].
     *
     * @param args The array of command-line arguments. args[0] is the program name,
     *             args[1] should be a string representing the desired index.
     * @return The argument at the specified index, or null if the index is invalid,
     *         malformed, or out of bounds.
     */
    public static String getArgumentByIndex(String[] args) {
        // Rule #4: Validate input - check if enough arguments are provided.
        if (args == null || args.length < 2) {
            System.err.println("Error: Not enough arguments. Please provide an index.");
            return null;
        }

        int index;
        try {
            // Rule #4: Validate input type.
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            // Rule #7: Handle exceptions for invalid number formats.
            System.err.println("Error: The first argument must be a valid integer index.");
            return null;
        }

        // Rule #1 & #4: Ensure the index is within the buffer's boundaries.
        if (index < 0 || index >= args.length) {
            System.err.println("Error: Index " + index + " is out of bounds.");
            return null;
        }

        return args[index];
    }

    public static void main(String[] args) {
        System.out.println("--- Running with actual command line arguments ---");
        String result = getArgumentByIndex(args);
        if (result != null) {
            System.out.println("Value at specified index: " + result);
        } else {
            System.out.println("Failed to retrieve argument.");
        }
        System.out.println("------------------------------------------------");
        
        runTests();
    }
    
    private static void runTests() {
        System.out.println("\n--- Running 5 test cases ---");

        // Test Case 1: Valid case
        System.out.println("\nTest Case 1: Valid index");
        String[] test1 = {"program", "2", "first", "second"};
        System.out.println("Input: " + Arrays.toString(test1));
        String result1 = getArgumentByIndex(test1);
        System.out.println("Output: " + (result1 != null ? result1 : "null"));

        // Test Case 2: Index out of bounds (too high)
        System.out.println("\nTest Case 2: Index out of bounds (high)");
        String[] test2 = {"program", "5", "only", "three", "args"};
        System.out.println("Input: " + Arrays.toString(test2));
        String result2 = getArgumentByIndex(test2);
        System.out.println("Output: " + (result2 != null ? result2 : "null"));

        // Test Case 3: Negative index
        System.out.println("\nTest Case 3: Negative index");
        String[] test3 = {"program", "-1", "a", "b"};
        System.out.println("Input: " + Arrays.toString(test3));
        String result3 = getArgumentByIndex(test3);
        System.out.println("Output: " + (result3 != null ? result3 : "null"));

        // Test Case 4: Invalid index format (not a number)
        System.out.println("\nTest Case 4: Invalid index format");
        String[] test4 = {"program", "foo", "bar"};
        System.out.println("Input: " + Arrays.toString(test4));
        String result4 = getArgumentByIndex(test4);
        System.out.println("Output: " + (result4 != null ? result4 : "null"));

        // Test Case 5: Not enough arguments
        System.out.println("\nTest Case 5: Not enough arguments");
        String[] test5 = {"program"};
        System.out.println("Input: " + Arrays.toString(test5));
        String result5 = getArgumentByIndex(test5);
        System.out.println("Output: " + (result5 != null ? result5 : "null"));
        
        System.out.println("\n--- Tests finished ---");
    }
}