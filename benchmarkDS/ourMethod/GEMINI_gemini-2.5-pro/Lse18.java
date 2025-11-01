public class Lse18 {

    private static final int[] DATA = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value from a predefined array at the given index.
     *
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of the valid range (0 to array.length-1).
     */
    public static int getValueAtIndex(int index) {
        // Rule #1, #4: Validate the index to ensure it is within the buffer's boundaries.
        if (index < 0 || index >= DATA.length) {
            // Rule #8: Fail closed with a clear error message.
            throw new ArrayIndexOutOfBoundsException("Index " + index + " is out of bounds for array of size " + DATA.length);
        }
        return DATA[index];
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testIndexes = {0, 5, 9, -1, 10};

        for (int index : testIndexes) {
            // Rule #7: Ensure all exceptions are caught and handled appropriately.
            try {
                int value = getValueAtIndex(index);
                System.out.println("Value at index " + index + ": " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.err.println("Error for index " + index + ": " + e.getMessage());
            }
        }
    }
}