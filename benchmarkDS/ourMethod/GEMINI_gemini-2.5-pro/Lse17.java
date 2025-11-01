import java.util.Arrays;

public class Lse17 {

    /**
     * Returns the value at a specified index in a given integer array.
     * This function performs bounds checking to prevent Out-of-bounds Read vulnerabilities.
     *
     * @param arr The array to access. Must not be null.
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of the array's bounds or the array is null.
     */
    public static int getValueAtIndex(int[] arr, int index) {
        // Rule #1, #4: Validate input index and array to ensure it is within the buffer's boundaries.
        if (arr == null) {
            throw new IndexOutOfBoundsException("Input array cannot be null.");
        }
        if (index < 0 || index >= arr.length) {
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array of length " + arr.length);
        }
        return arr[index];
    }

    public static void main(String[] args) {
        // The array from which to retrieve a value.
        final int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        
        // 5 test cases, including valid, edge, and invalid indices.
        int[] testIndices = {0, 5, 9, -1, 10};

        System.out.println("Running test cases for array: " + Arrays.toString(data));
        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("\n--- Test Case " + (i + 1) + ": Attempting to access index " + index + " ---");
            try {
                // Rule #7: Handle exceptions appropriately.
                int value = getValueAtIndex(data, index);
                System.out.println("Success: Value at index " + index + " is " + value);
            } catch (IndexOutOfBoundsException e) {
                // Rule #8: Provide clear, minimal error messages.
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}