import java.util.Arrays;

public class Lse5 {

    /**
     * Finds and prints an element from an array based on a string index argument.
     * It handles parsing and validation of the index.
     *
     * @param data The array of strings to access.
     * @param args An array containing the string representation of the index at args[0].
     */
    public static void printElementAtIndex(String[] data, String[] args) {
        // 1. Security Check: Ensure an argument is provided.
        if (args == null || args.length < 1) {
            System.out.println("Error: No index provided.");
            System.out.println("Usage: please provide an integer index as an argument.");
            return;
        }

        String arg = args[0];
        int index;

        // 2. Security Check: Ensure the argument is a valid integer.
        try {
            index = Integer.parseInt(arg);
        } catch (NumberFormatException e) {
            System.out.println("Error: Argument '" + arg + "' is not a valid integer.");
            return;
        }

        // 3. Security Check: Ensure the index is within the array bounds.
        if (index < 0 || index >= data.length) {
            System.out.println("Error: Index " + index + " is out of bounds. Must be between 0 and " + (data.length - 1) + ".");
            return;
        }

        // If all checks pass, print the element.
        System.out.println("Element at index " + index + ": " + data[index]);
    }

    public static void main(String[] args) {
        // This program is intended to be run with command-line arguments.
        // For example: java Lse5 2
        // To demonstrate functionality, we will simulate command-line arguments for 5 test cases.

        String[] data = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid index
        System.out.print("Test Case 1 (Input: \"2\"): ");
        printElementAtIndex(data, new String[]{"2"});

        // Test Case 2: Out-of-bounds index (too high)
        System.out.print("Test Case 2 (Input: \"10\"): ");
        printElementAtIndex(data, new String[]{"10"});

        // Test Case 3: Out-of-bounds index (negative)
        System.out.print("Test Case 3 (Input: \"-1\"): ");
        printElementAtIndex(data, new String[]{"-1"});

        // Test Case 4: Non-numeric input
        System.out.print("Test Case 4 (Input: \"abc\"): ");
        printElementAtIndex(data, new String[]{"abc"});

        // Test Case 5: No input
        System.out.print("Test Case 5 (Input: empty): ");
        printElementAtIndex(data, new String[]{});

        System.out.println("\n--- Testing with actual command-line arguments ---");
        // This part will use the actual arguments passed when running the program
        if (args.length > 0) {
            System.out.println("Actual command-line input: " + Arrays.toString(args));
            printElementAtIndex(data, args);
        } else {
             System.out.println("No actual command-line arguments provided. Run like 'java Lse5 3'");
        }
    }
}