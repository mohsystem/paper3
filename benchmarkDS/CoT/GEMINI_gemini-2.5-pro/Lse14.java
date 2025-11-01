public class Lse14 {

    // A predefined constant array of IDs.
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Retrieves the value from id_sequence at a specified index.
     *
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of the valid range (0 to length-1).
     */
    public static int getValueAtIndex(int index) {
        // Security Check: Validate that the index is within the bounds of the array.
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            // If the index is invalid, throw an exception to indicate the error.
            throw new IndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        int[] testIndexes = {3, 0, 9, -1, 10};

        for (int i = 0; i < testIndexes.length; i++) {
            int indexToTest = testIndexes[i];
            System.out.println("Test Case " + (i + 1) + ": Testing index " + indexToTest);
            try {
                int value = getValueAtIndex(indexToTest);
                System.out.println(" -> Success: Value at index " + indexToTest + " is " + value);
            } catch (IndexOutOfBoundsException e) {
                System.out.println(" -> " + e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}