public class Lse14 {

    private static final int[] id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Returns the value at the specified index in the id_sequence array.
     *
     * @param index The index of the element to return.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of range
     *         (index < 0 || index >= id_sequence.length).
     */
    public static int getValueAtIndex(int index) {
        // Rule #1: Ensure all input is validated.
        // Rule #3: Ensure that all buffer operations strictly adhere to the buffer's boundaries.
        if (index < 0 || index >= id_sequence.length) {
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for the sequence of length " + id_sequence.length);
        }
        return id_sequence[index];
    }

    /**
     * Main method to demonstrate the functionality of getValueAtIndex with 5 test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // 5 test cases
        int[] testIndices = {0, 9, 5, -1, 10};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("Testing with index: " + index);
            try {
                // Rule #5: Ensure that all exceptions are caught and handled appropriately.
                int value = getValueAtIndex(index);
                System.out.println("Value at index " + index + ": " + value);
            } catch (IndexOutOfBoundsException e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}