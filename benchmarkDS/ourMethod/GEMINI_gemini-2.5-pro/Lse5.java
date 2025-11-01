public class Lse5 {

    private static final int[] DATA = {100, 200, 300, 400, 500};

    /**
     * Parses a string index from args, validates it, and prints the element from DATA.
     * @param args Command-line arguments, expected to contain a single integer string.
     */
    public static void printElementAtIndex(String[] args) {
        if (args == null || args.length != 1) {
            System.err.println("Usage: java Lse5 <index>");
            return;
        }

        try {
            int index = Integer.parseInt(args[0]);

            // Explicitly check if the index is within the valid bounds of the array.
            if (index < 0 || index >= DATA.length) {
                System.err.println("Error: Index " + index + " is out of bounds. Valid range is 0 to " + (DATA.length - 1) + ".");
                return;
            }

            System.out.println("Element at index " + index + ": " + DATA[index]);

        } catch (NumberFormatException e) {
            System.err.println("Error: Argument must be a valid integer.");
        }
    }

    public static void main(String[] args) {
        // This block allows the program to be run with actual command-line arguments.
        if (args.length > 0) {
            System.out.println("--- Running with actual command-line arguments ---");
            printElementAtIndex(args);
            System.out.println("-----------------------------------------------\n");
        }

        // Running 5 test cases as requested.
        System.out.println("--- Running 5 built-in test cases ---");

        System.out.println("\n[Test 1] Valid index '2':");
        printElementAtIndex(new String[]{"2"});

        System.out.println("\n[Test 2] Out of bounds index '10':");
        printElementAtIndex(new String[]{"10"});

        System.out.println("\n[Test 3] Negative index '-1':");
        printElementAtIndex(new String[]{"-1"});

        System.out.println("\n[Test 4] Non-numeric input 'abc':");
        printElementAtIndex(new String[]{"abc"});

        System.out.println("\n[Test 5] No arguments:");
        printElementAtIndex(new String[]{});

        System.out.println("\n--- Test cases finished ---");
    }
}