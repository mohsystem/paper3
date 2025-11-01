public class Lse13 {
    // A predefined sequence of IDs
    private static final int[] id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

    /**
     * Returns the value at the given index in the id_sequence.
     *
     * @param index The index of the value to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            return -1; // Return -1 to indicate an invalid index
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Valid index at the beginning
        int index1 = 0;
        System.out.println("Value at index " + index1 + ": " + getValueAtIndex(index1));

        // Test Case 2: Valid index in the middle
        int index2 = 4;
        System.out.println("Value at index " + index2 + ": " + getValueAtIndex(index2));

        // Test Case 3: Valid index at the end
        int index3 = 9;
        System.out.println("Value at index " + index3 + ": " + getValueAtIndex(index3));

        // Test Case 4: Invalid index (out of bounds high)
        int index4 = 10;
        System.out.println("Value at index " + index4 + ": " + getValueAtIndex(index4));

        // Test Case 5: Invalid index (negative)
        int index5 = -1;
        System.out.println("Value at index " + index5 + ": " + getValueAtIndex(index5));
    }
}