public class Lse17 {

    private static final int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Securely retrieves the value at a given index from a predefined array.
     * This function performs bounds checking to prevent out-of-bounds access.
     *
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of the valid range.
     */
    public static int getValueAtIndex(int index) {
        // Secure: Check if the index is within the valid bounds of the array.
        if (index >= 0 && index < data.length) {
            return data[index];
        } else {
            // If the index is invalid, throw an exception to signal the error.
            throw new IndexOutOfBoundsException("Error: Index " + index + " is out of bounds.");
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases: 3 valid, 2 invalid
        int[] testIndexes = {3, 0, 9, -1, 10};

        System.out.println("--- Java: Testing getValueAtIndex ---");
        for (int index : testIndexes) {
            try {
                int value = getValueAtIndex(index);
                System.out.println("Test Case: index = " + index);
                System.out.println("  -> Success: Value is " + value);
            } catch (IndexOutOfBoundsException e) {
                System.out.println("Test Case: index = " + index);
                System.out.println("  -> " + e.getMessage());
            }
            System.out.println("-------------------------------------");
        }
    }
}