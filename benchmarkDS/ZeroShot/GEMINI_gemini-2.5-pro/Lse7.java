import java.util.Arrays;

class Lse7 {

    /**
     * Converts the second command-line argument to an integer (INDEX) and prints
     * the argument at that INDEX.
     *
     * @param args The array of command-line arguments.
     */
    public static void processAndPrintArgument(String[] args) {
        // 1. Check if the second argument (for the index) exists.
        if (args.length < 2) {
            System.out.println("Error: Not enough arguments. Please provide an index.");
            return;
        }

        String indexStr = args[1];
        int index;

        // 2. Securely convert the string to an integer.
        try {
            index = Integer.parseInt(indexStr);
        } catch (NumberFormatException e) {
            System.out.println("Error: The provided index '" + indexStr + "' is not a valid integer or is out of the integer range.");
            return;
        }

        // 3. Check if the parsed index is within the valid bounds of the arguments array.
        if (index < 0 || index >= args.length) {
            System.out.println("Error: Index " + index + " is out of bounds. Must be between 0 and " + (args.length - 1) + ".");
            return;
        }

        // 4. Print the argument at the specified index.
        System.out.println("Argument at index " + index + ": " + args[index]);
    }

    public static void main(String[] args) {
        System.out.println("--- Test Cases ---");

        // Test Case 1: Valid input
        // Expected: Argument at index 2: Hello
        System.out.println("\nTest Case 1: java Lse7 2 Hello World");
        String[] test1 = {"Lse7", "2", "Hello", "World"};
        processAndPrintArgument(test1);

        // Test Case 2: Index out of bounds (too high)
        // Expected: Error: Index 10 is out of bounds.
        System.out.println("\nTest Case 2: java Lse7 10 Hello");
        String[] test2 = {"Lse7", "10", "Hello"};
        processAndPrintArgument(test2);

        // Test Case 3: Invalid number format for index
        // Expected: Error: The provided index 'abc' is not a valid integer...
        System.out.println("\nTest Case 3: java Lse7 abc Hello");
        String[] test3 = {"Lse7", "abc", "Hello"};
        processAndPrintArgument(test3);

        // Test Case 4: Not enough arguments to get the index
        // Expected: Error: Not enough arguments.
        System.out.println("\nTest Case 4: java Lse7");
        String[] test4 = {"Lse7"};
        processAndPrintArgument(test4);

        // Test Case 5: Integer overflow for the index
        // Expected: Error: The provided index '2147483648' is not a valid integer...
        System.out.println("\nTest Case 5: java Lse7 2147483648");
        String[] test5 = {"Lse7", "2147483648"};
        processAndPrintArgument(test5);
    }
}