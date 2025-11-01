public class Lse12 {

    /**
     * Gets the value from an array at a specified index.
     *
     * @param arr The source array of integers.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The integer value at the given index.
     * @throws IndexOutOfBoundsException if the index is out of the valid range [0, size-1].
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Security check: Ensure the index is within the valid bounds of the array.
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Error: Index " + index + " is out of bounds for array size " + size);
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] testArray = {10, 20, 30, 40, 50};
        int size = testArray.length;

        // Test cases
        int[] testIndexes = {2, 0, 4, -1, 5};

        System.out.println("Running Java Test Cases...");
        for (int i = 0; i < testIndexes.length; i++) {
            int indexToTest = testIndexes[i];
            System.out.println("Test Case " + (i + 1) + ": Trying to get value at index " + indexToTest);
            try {
                int value = getValueFromArray(testArray, size, indexToTest);
                System.out.println("  Success: Value is " + value);
            } catch (IndexOutOfBoundsException e) {
                System.out.println("  Caught expected exception: " + e.getMessage());
            }
        }
    }
}