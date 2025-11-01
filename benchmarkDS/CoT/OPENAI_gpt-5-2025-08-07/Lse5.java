// Chain-of-Through process:
// 1) Problem understanding:
//    - Create an array of elements.
//    - Read a command-line argument, convert it to an integer index.
//    - Print the element at that index.
//    - Provide a function that accepts inputs as parameters and returns the element (if valid).
// 2) Security requirements:
//    - Validate that an argument exists.
//    - Parse the argument safely to an integer, handling invalid formats.
//    - Check index bounds (non-negative and within array length).
// 3) Secure coding generation:
//    - Use Integer.parseInt with try/catch to handle bad inputs.
//    - Perform strict bounds checks before accessing the array.
//    - Avoid exposing internal errors; provide clear error messages.
// 4) Code review (summary):
//    - All inputs validated before use.
//    - No unchecked array accesses.
//    - No resource leaks or unsafe operations.
//    - Errors handled gracefully with messages to stderr.
// 5) Secure code output:
//    - Final code applies the above checks and prints the element or a clear error.
//
// Main class name requirement: Lse5
public class Lse5 {

    // Elements array (non-empty, fixed size)
    private static final String[] ELEMENTS = new String[] { "alpha", "beta", "gamma", "delta", "epsilon" };

    // Function: accepts inputs as parameters and returns the output if any.
    // Returns the element at the given index, or null if invalid.
    public static String elementAtIndex(String[] elements, int index) {
        if (elements == null) {
            return null;
        }
        if (index < 0 || index >= elements.length) {
            return null;
        }
        return elements[index];
    }

    // Helper to process a single argument safely and print result
    private static void processArgument(String arg) {
        if (arg == null) {
            System.err.println("Error: No index argument provided.");
            return;
        }
        final int idx;
        try {
            idx = Integer.parseInt(arg.trim());
        } catch (NumberFormatException e) {
            System.err.println("Error: Index must be a valid integer.");
            return;
        }

        String result = elementAtIndex(ELEMENTS, idx);
        if (result == null) {
            System.err.println("Error: Index out of bounds. Valid range is 0 to " + (ELEMENTS.length - 1) + ".");
        } else {
            System.out.println(result);
        }
    }

    // Main method with 5 test cases when no CLI arg is provided
    public static void main(String[] args) {
        if (args != null && args.length >= 1) {
            processArgument(args[0]);
        } else {
            // 5 test cases
            int[] tests = new int[] { 0, 2, 4, -1, 5 };
            for (int t : tests) {
                String result = elementAtIndex(ELEMENTS, t);
                if (result == null) {
                    System.err.println("Test index " + t + ": Error - Index out of bounds. Valid range is 0 to " + (ELEMENTS.length - 1) + ".");
                } else {
                    System.out.println("Test index " + t + ": " + result);
                }
            }
        }
    }
}