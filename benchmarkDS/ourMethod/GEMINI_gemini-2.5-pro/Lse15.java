public class Lse15 {

    private static final int[] ID_SEQUENCE = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Returns the value at a given index in the ID_SEQUENCE array.
     * This function demonstrates secure coding practices by validating the index
     * to prevent out-of-bounds access.
     *
     * @param index The index of the value to retrieve.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is outside the valid range.
     */
    public static int getValueAtIndex(int index) {
        // Rules #1, #2, #4: Ensure that the index is within the buffer's boundaries.
        if (index < 0 || index >= ID_SEQUENCE.length) {
            // Rule #7: Use exceptions for handling errors.
            throw new IndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
        return ID_SEQUENCE[index];
    }

    /**
     * A helper method to run a single test case and print the result.
     * @param index The index to test.
     */
    private static void runTestCase(String testName, int index) {
        System.out.print(testName + " with index " + index + ": ");
        try {
            int value = getValueAtIndex(index);
            System.out.println("Success! Value is " + value);
        } catch (IndexOutOfBoundsException e) {
            System.out.println(e.getMessage());
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");
        // Test Case 1: Valid index (start of array)
        runTestCase("Test Case 1", 0);
        // Test Case 2: Valid index (middle of array)
        runTestCase("Test Case 2", 5);
        // Test Case 3: Valid index (end of array)
        runTestCase("Test Case 3", 9);
        // Test Case 4: Invalid index (negative)
        runTestCase("Test Case 4", -1);
        // Test Case 5: Invalid index (out of bounds)
        runTestCase("Test Case 5", ID_SEQUENCE.length);

        // The prompt "The function first prompts the user for an index value" is interpreted
        // as the main application logic prompting the user before calling the function.
        System.out.println("\n--- Interactive User Test ---");
        // Rule #7: Use try-with-resources to manage the Scanner resource.
        try (java.util.Scanner scanner = new java.util.Scanner(System.in)) {
            System.out.print("Please enter an index value: ");
            try {
                // Rule #4: Validate that the input is of the expected type.
                int userIndex = scanner.nextInt();
                int value = getValueAtIndex(userIndex);
                System.out.println("The value at index " + userIndex + " is: " + value);
            } catch (java.util.InputMismatchException e) {
                System.out.println("Error: Invalid input. Please enter an integer.");
            } catch (IndexOutOfBoundsException e) {
                System.out.println(e.getMessage());
            }
        }
    }
}