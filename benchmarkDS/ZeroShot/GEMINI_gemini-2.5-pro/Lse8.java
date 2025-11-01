public class Lse8 {

    /**
     * Securely retrieves an argument from the array based on an index provided in args[1].
     *
     * @param args The array of command-line arguments. args[1] is expected to be the index.
     * @return The string at the specified index or an error message if any validation fails.
     */
    public static String getArgumentAtIndex(String[] args) {
        // Security Check 1: Ensure there are enough arguments to get an index.
        if (args.length < 2) {
            return "Error: Not enough arguments. An index must be provided as the second argument.";
        }

        int index;
        try {
            // Security Check 2: Parse the index, handle non-integer input.
            index = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            return "Error: Index provided in the second argument must be a valid integer.";
        }

        // Security Check 3: Ensure the index is within the valid bounds of the array.
        if (index < 0) {
            return "Error: Index cannot be negative.";
        }
        if (index >= args.length) {
            return "Error: Index " + index + " is out of bounds for " + args.length + " arguments.";
        }

        // If all checks pass, return the value at the specified index.
        return args[index];
    }

    public static void main(String[] args) {
        // Main function runs 5 test cases by simulating command-line arguments.
        System.out.println("--- Java: Testing argument retrieval ---");

        // Test Case 1: Valid input
        String[] test1 = {"Lse8", "3", "apple", "banana"};
        System.out.println("Test Case 1 Input: {\"Lse8\", \"3\", \"apple\", \"banana\"}");
        System.out.println("Result: " + getArgumentAtIndex(test1) + "\n");

        // Test Case 2: Not enough arguments
        String[] test2 = {"Lse8"};
        System.out.println("Test Case 2 Input: {\"Lse8\"}");
        System.out.println("Result: " + getArgumentAtIndex(test2) + "\n");

        // Test Case 3: Invalid index format (not an integer)
        String[] test3 = {"Lse8", "abc", "arg2", "arg3"};
        System.out.println("Test Case 3 Input: {\"Lse8\", \"abc\", \"arg2\", \"arg3\"}");
        System.out.println("Result: " + getArgumentAtIndex(test3) + "\n");
        
        // Test Case 4: Index out of bounds (too high)
        String[] test4 = {"Lse8", "10", "arg2"};
        System.out.println("Test Case 4 Input: {\"Lse8\", \"10\", \"arg2\"}");
        System.out.println("Result: " + getArgumentAtIndex(test4) + "\n");

        // Test Case 5: Index out of bounds (negative)
        String[] test5 = {"Lse8", "-1", "arg2", "arg3"};
        System.out.println("Test Case 5 Input: {\"Lse8\", \"-1\", \"arg2\", \"arg3\"}");
        System.out.println("Result: " + getArgumentAtIndex(test5));
    }
}