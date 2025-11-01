public class Lse14 {
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value at the given index in the id_sequence array.
     * @param index The index of the value to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            return -1; // Indicate index out of bounds
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testIndices = {0, 3, 9, -1, 10};

        System.out.println("Testing getValueAtIndex function:");
        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            int value = getValueAtIndex(index);
            System.out.println("Value at index " + index + ": " + value);
        }
    }
}