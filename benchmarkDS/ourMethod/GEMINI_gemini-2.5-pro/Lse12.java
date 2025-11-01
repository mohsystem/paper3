import java.util.InputMismatchException;
import java.util.Scanner;

public class Lse12 {

    /**
     * Returns the value at a given index in an array.
     * Performs bounds checking to prevent out-of-bounds access.
     *
     * @param arr   The array of integers.
     * @param size  The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws IllegalArgumentException if the index is out of bounds or array is null.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Rule #1, #4: Validate inputs and check boundaries
        if (arr == null || index < 0 || index >= size) {
            throw new IllegalArgumentException("Index is out of bounds or array is null.");
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50};
        int dataSize = data.length;
        int exitCode = 1; // Default exit code for error cases

        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");
        runTestCase(data, dataSize, 0, 10);
        runTestCase(data, dataSize, 4, 50);
        runTestCase(data, dataSize, 2, 30);
        runTestCase(data, dataSize, -1, -1); // Expected: Exception
        runTestCase(data, dataSize, 5, -1);  // Expected: Exception
        System.out.println("--- Test Cases Finished ---\n");

        System.out.println("Enter an index to retrieve a value from the array:");
        // Rule #7: Use try-with-resources for automatic resource management
        try (Scanner scanner = new Scanner(System.in)) {
            int index = scanner.nextInt();
            int value = getValueFromArray(data, dataSize, index);
            System.out.println("Value at index " + index + " is " + value);
            exitCode = value;
        } catch (InputMismatchException e) {
            // Rule #4, #7: Handle invalid input type
            System.err.println("Error: Invalid input. Please enter an integer.");
            exitCode = 1;
        } catch (IllegalArgumentException e) {
            // Rule #7: Handle exceptions from the function call
            System.err.println("Error: " + e.getMessage());
            exitCode = 1;
        }

        System.exit(exitCode);
    }

    /**
     * Helper function to run a single test case and print the result.
     */
    private static void runTestCase(int[] arr, int size, int index, int expected) {
        System.out.print("Test case: index=" + index + ". ");
        try {
            int result = getValueFromArray(arr, size, index);
            if (result == expected) {
                System.out.println("Result: " + result + ". PASS");
            } else {
                System.out.println("Result: " + result + ". FAIL (Expected: " + expected + ")");
            }
        } catch (IllegalArgumentException e) {
            // Assuming -1 indicates an expected exception for this test helper
            if (expected == -1) {
                System.out.println("Caught expected exception. PASS");
            } else {
                System.out.println("Caught unexpected exception: " + e.getMessage() + ". FAIL");
            }
        }
    }
}