import java.util.Arrays;

public class Lse11 {

    /**
     * Gets a value from an array at a specific index with bounds checking.
     * The function prevents out-of-bounds access by validating the index.
     *
     * @param arr The source array of integers.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the given index, or 1 as an error code if the index is out of bounds.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Security: Check if the index is within the valid bounds of the array (0 to size-1).
        // This prevents ArrayIndexOutOfBoundsException.
        if (index < 0 || index >= size) {
            System.err.println("Error: Index " + index + " is out of bounds for array size " + size + ".");
            return 1; // Return an error code for out-of-bounds access.
        }
        return arr[index];
    }

    /**
     * Main method to demonstrate getValueFromArray with 5 test cases.
     * The original prompt required reading a single value and exiting with the result code.
     * To show 5 test cases in a single run, this main method simulates those runs
     * and prints what the result/exit code would be.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50};
        int size = id_sequence.length;

        // 5 Test Cases to demonstrate the functionality and security checks.
        int[] testIndices = {
            0,  // Test Case 1: Valid index at the beginning
            2,  // Test Case 2: Valid index in the middle
            4,  // Test Case 3: Valid index at the end
            -1, // Test Case 4: Invalid index (negative)
            5   // Test Case 5: Invalid index (out of bounds high)
        };

        System.out.println("Running 5 test cases for id_sequence: " + Arrays.toString(id_sequence));
        System.out.println("------------------------------------------");

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("Test Case " + (i + 1) + ": Requesting index " + index);
            
            int result = getValueFromArray(id_sequence, size, index);
            
            System.out.println(" -> Function returned: " + result);
            if (result == 1 && (index < 0 || index >= size)) {
                System.out.println("   (This would be an exit code indicating an error)");
            } else {
                System.out.println("   (Value at index " + index + " is " + result + ". This would be the exit code)");
            }
            System.out.println("------------------------------------------");
        }
    }
}