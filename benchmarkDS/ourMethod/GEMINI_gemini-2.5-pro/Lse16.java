public class Lse16 {
    private static final int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value at the specified index in a predefined array.
     *
     * @param index The index of the element to return.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of range
     *         (index < 0 || index >= array.length).
     */
    public static int getValueAtIndex(int index) {
        // Input validation: Check if the index is within the bounds of the array.
        if (index < 0 || index >= data.length) {
            // Fail closed with a clear error message.
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array of size " + data.length);
        }
        return data[index];
    }

    public static void main(String[] args) {
        // 5 Test Cases
        int[] testIndices = {0, 5, 9, -1, 10};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("Test Case " + (i + 1) + ": Trying to access index " + index);
            try {
                int value = getValueAtIndex(index);
                System.out.println(" -> Success: Value at index " + index + " is " + value);
            } catch (IndexOutOfBoundsException e) {
                System.out.println(" -> Caught expected exception: " + e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}