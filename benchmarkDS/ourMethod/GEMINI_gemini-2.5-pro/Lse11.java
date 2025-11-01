import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse11 {

    /**
     * Returns the value from the array at the given index.
     * This function performs bounds checking to prevent out-of-bounds access.
     *
     * @param array The array of integers.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the given index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of bounds.
     */
    public static int getValueFromArray(int[] array, int size, int index) {
        // Rule #1: Ensure that all buffer operations strictly adhere to the buffer's boundaries.
        if (index < 0 || index >= size) {
            throw new ArrayIndexOutOfBoundsException("Index " + index + " is out of bounds for size " + size);
        }
        return array[index];
    }

    /**
     * Main method to read an index from stdin and use it to access an array.
     * The value from the array is returned as the program's exit code.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        final int size = id_sequence.length;

        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");
        runTest(id_sequence, size, 0, 10);   // Test case 1: Valid, first element
        runTest(id_sequence, size, 9, 100);  // Test case 2: Valid, last element
        runTest(id_sequence, size, -1, 1);   // Test case 3: Invalid, negative index
        runTest(id_sequence, size, 10, 1);   // Test case 4: Invalid, out of bounds
        // Test case 5 (invalid format) is implicitly handled by the interactive part
        System.out.println("--- Test Cases Finished ---");
        
        System.out.println("\nEnter an integer index:");

        try (Scanner scanner = new Scanner(System.in)) {
            // Rule #4: Ensure all input is validated.
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input. Please enter an integer.");
                System.exit(1);
            }
            int index = scanner.nextInt();
            
            // Rule #7: Ensure that all exceptions are caught and handled.
            try {
                int value = getValueFromArray(id_sequence, size, index);
                System.out.println("Value at index " + index + " is " + value + ". Exiting with this value.");
                System.exit(value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } catch (Exception e) {
             System.err.println("An unexpected error occurred.");
             System.exit(1);
        }
    }

    private static void runTest(int[] array, int size, int index, int expectedExitCode) {
        System.out.print("Test with index " + index + ": ");
        int exitCode;
        try {
            exitCode = getValueFromArray(array, size, index);
        } catch (ArrayIndexOutOfBoundsException e) {
            exitCode = 1; // Simulate error exit code
        }
        
        if (exitCode == expectedExitCode) {
            System.out.println("PASSED (Expected exit code: " + expectedExitCode + ", Got: " + exitCode + ")");
        } else {
            System.out.println("FAILED (Expected exit code: " + expectedExitCode + ", Got: " + exitCode + ")");
        }
    }
}