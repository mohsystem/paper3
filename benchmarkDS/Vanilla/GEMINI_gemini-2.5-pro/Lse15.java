public class Lse15 {
    // The predefined array
    private static final int[] id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Returns the value at the given index in the id_sequence array.
     * @param index The index of the value to retrieve.
     * @return The integer value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is invalid.
     */
    public static int getValueAtIndex(int index) {
        // The prompt for user input is handled in main for testing purposes.
        // This function directly takes the index as a parameter.
        if (index < 0 || index >= id_sequence.length) {
            throw new ArrayIndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
        return id_sequence[index];
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testIndices = {0, 4, 9, -1, 10};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("Test Case " + (i + 1) + ": Getting value at index " + index);
            try {
                int value = getValueAtIndex(index);
                System.out.println("Value: " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println(e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}