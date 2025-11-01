public class Lse16 {

    // A pre-defined constant array.
    private static final int[] PREDEFINED_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value at the given index from a predefined array.
     *
     * @param index The integer index to look up.
     * @return The value at the specified index. If the index is out of bounds,
     *         it returns Integer.MIN_VALUE to signify an error.
     */
    public static int getValueAtIndex(int index) {
        // Security Check: Ensure the index is within the valid bounds of the array.
        // This prevents ArrayIndexOutOfBoundsException.
        if (index < 0 || index >= PREDEFINED_ARRAY.length) {
            // Return a special value to indicate an error.
            return Integer.MIN_VALUE;
        }
        // If the index is valid, return the element at that position.
        return PREDEFINED_ARRAY[index];
    }

    /**
     * Main method with 5 test cases to demonstrate the functionality.
     */
    public static void main(String[] args) {
        // Test cases including valid, edge, and invalid indices.
        int[] testCases = {5, 0, 9, -1, 10};

        System.out.println("Running Java Test Cases...");
        for (int index : testCases) {
            System.out.println("Testing index: " + index);
            int result = getValueAtIndex(index);

            // Check for the error value.
            if (result == Integer.MIN_VALUE) {
                System.out.println("Error: Index is out of bounds.");
            } else {
                System.out.println("Value at index " + index + ": " + result);
            }
            System.out.println("---");
        }
    }
}