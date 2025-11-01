class Lse13 {
    // A predefined sequence of IDs.
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Retrieves the value from id_sequence at a specified index.
     *
     * @param index The index of the value to retrieve.
     * @return The integer value at the given index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of the valid range.
     */
    public static int getValueAtIndex(int index) {
        // Security: Check if the index is within the valid bounds of the array.
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            // If the index is invalid, throw an exception.
            throw new ArrayIndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
    }

    public static void main(String[] args) {
        // 5 test cases for the getValueAtIndex function.
        int[] testIndexes = {0, 4, 9, -1, 10};

        System.out.println("--- Running Java Test Cases ---");
        for (int index : testIndexes) {
            try {
                int value = getValueAtIndex(index);
                System.out.println("Value at index " + index + ": " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("Attempted to access index " + index + ". " + e.getMessage());
            }
        }
    }
}