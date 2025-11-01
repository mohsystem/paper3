public class Lse5 {

    private static final int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Securely accesses an element in a predefined array based on a string index.
     * It handles parsing errors and bounds checking.
     *
     * @param indexStr The string representation of the index to access.
     */
    public static void processIndex(String indexStr) {
        try {
            int index = Integer.parseInt(indexStr);

            // Secure bounds checking
            if (index >= 0 && index < data.length) {
                System.out.println("Element at index " + index + ": " + data[index]);
            } else {
                System.err.println("Error: Index " + index + " is out of bounds.");
            }
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid input. Please provide a valid integer.");
        }
    }

    /**
     * Runs a series of predefined test cases.
     */
    public static void runTests() {
        System.out.println("--- Running Test Cases ---");
        
        // Test Case 1: Valid index
        System.out.print("Test 1 (Input: \"3\"): ");
        processIndex("3");

        // Test Case 2: Negative index (out of bounds)
        System.out.print("Test 2 (Input: \"-1\"): ");
        processIndex("-1");
        
        // Test Case 3: Index equal to array size (out of bounds)
        System.out.print("Test 3 (Input: \"10\"): ");
        processIndex("10");

        // Test Case 4: Last valid index
        System.out.print("Test 4 (Input: \"9\"): ");
        processIndex("9");
        
        // Test Case 5: Non-numeric input
        System.out.print("Test 5 (Input: \"abc\"): ");
        processIndex("abc");
        
        System.out.println("--- Test Cases Finished ---");
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            // Process the command line argument
            processIndex(args[0]);
        } else {
            // Run built-in test cases if no command line argument is provided
            System.out.println("Usage: java Lse5 <index>");
            System.out.println("No command line argument provided. Running default test cases.");
            runTests();
        }
    }
}