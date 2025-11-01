public class Lse13 {

    // The predefined array of IDs.
    private static final int[] id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Retrieves the value from id_sequence at a specified index.
     * This method is secure against array index out-of-bounds errors.
     *
     * @param index The index of the value to retrieve.
     * @return The value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is negative or greater than or equal to the array length.
     */
    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= id_sequence.length) {
            // Throw an exception for invalid index access, which is the standard Java practice.
            throw new ArrayIndexOutOfBoundsException("Error: Index " + index + " is out of bounds for the array of size " + id_sequence.length + ".");
        }
        return id_sequence[index];
    }

    /**
     * Main method with 5 test cases to demonstrate the functionality and security of getValueAtIndex.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Test cases: valid start, valid middle, valid end, invalid negative, invalid positive
        int[] testCases = {0, 4, 9, -1, 10};

        System.out.println("--- Running Java Test Cases ---");
        for (int index : testCases) {
            try {
                int value = getValueAtIndex(index);
                System.out.println("SUCCESS: Value at index " + index + " is " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("FAILURE: Attempted to access index " + index + ". " + e.getMessage());
            }
        }
    }
}