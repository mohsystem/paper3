import java.util.InputMismatchException;

public class Lse14 {

    // A final array to prevent modification.
    private static final int[] id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Retrieves the value from id_sequence at a specified index.
     * This method is secure because it performs bounds checking.
     *
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of range
     *         (index < 0 || index >= id_sequence.length).
     */
    public static int getValueAtIndex(int index) {
        // Security check: Ensure the index is within the valid bounds of the array.
        if (index < 0 || index >= id_sequence.length) {
            // Throwing an exception is the standard Java way to handle this error.
            throw new ArrayIndexOutOfBoundsException("Index " + index + " is out of bounds for sequence of length " + id_sequence.length);
        }
        return id_sequence[index];
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testIndexes = {0, 9, 4, -1, 10};

        System.out.println("Running 5 test cases...");
        for (int i = 0; i < testIndexes.length; i++) {
            int index = testIndexes[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Attempting to get value at index: " + index);
            try {
                int value = getValueAtIndex(index);
                System.out.println("Success! Value at index " + index + " is: " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}