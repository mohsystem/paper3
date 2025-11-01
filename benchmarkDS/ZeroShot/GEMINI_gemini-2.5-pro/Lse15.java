public class Lse15 {

    // A private, static, and final array to prevent modification.
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Securely gets a value from the id_sequence array at a given index.
     * It performs bounds checking to prevent unauthorized memory access.
     *
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws ArrayIndexOutOfBoundsException if the index is out of range.
     */
    public static int getValueAtIndex(int index) {
        // Secure: Check if the index is within the valid bounds of the array.
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            // If the index is invalid, throw an exception with a clear message.
            throw new ArrayIndexOutOfBoundsException("Error: Index " + index + " is out of bounds. Must be between 0 and " + (id_sequence.length - 1) + ".");
        }
    }

    public static void main(String[] args) {
        // 5 test cases for the getValueAtIndex function.
        int[] testIndexes = {0, 5, 9, -1, 10};

        System.out.println("Running 5 test cases...");

        for (int i = 0; i < testIndexes.length; i++) {
            int index = testIndexes[i];
            System.out.println("\n--- Test Case " + (i + 1) + ": Trying index " + index + " ---");
            try {
                int value = getValueAtIndex(index);
                System.out.println("Success: Value at index " + index + " is " + value);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println(e.getMessage());
            }
        }
    }
}