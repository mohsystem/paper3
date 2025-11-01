import java.util.InputMismatchException;

public class Lse13 {

    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value at the specified index in the id_sequence array.
     *
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of range
     *         (index < 0 || index >= id_sequence.length).
     */
    public static int getValueAtIndex(int index) {
        // Rule #1, #4: Ensure that buffer operations strictly adhere to the buffer's boundaries.
        if (index < 0 || index >= id_sequence.length) {
            throw new ArrayIndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
        return id_sequence[index];
    }

    /**
     * Main method to run test cases.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // 5 test cases
        int[] testIndices = {0, 5, 9, -1, 10};

        System.out.println("Running 5 test cases...");
        for (int i = 0; i < testIndices.length; i++) {
            int testIndex = testIndices[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Attempting to access index: " + testIndex);
            try {
                // Rule #7: Ensure that all exceptions are caught and handled appropriately.
                int value = getValueAtIndex(testIndex);
                System.out.println("Value at index " + testIndex + ": " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.err.println(e.getMessage());
            }
        }
    }
}