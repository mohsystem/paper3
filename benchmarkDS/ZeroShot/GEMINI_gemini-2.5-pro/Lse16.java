public class Lse16 {

    /**
     * Securely gets a value from an array at a given index.
     * It performs bounds checking to prevent IndexOutOfBoundsException.
     *
     * @param arr   The source integer array.
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of the array's bounds.
     * @throws IllegalArgumentException if the input array is null.
     */
    public static int getValueAtIndex(int[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        if (index >= 0 && index < arr.length) {
            return arr[index];
        } else {
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array of size " + arr.length);
        }
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

        // 5 Test cases: valid middle, valid start, valid end, invalid negative, invalid positive
        int[] testIndexes = {5, 0, 9, -1, 10};

        System.out.println("Testing in Java...");
        System.out.println("Array: [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]");
        for (int index : testIndexes) {
            try {
                int value = getValueAtIndex(data, index);
                System.out.println("Test case index " + index + ": Success! Value is " + value);
            } catch (IndexOutOfBoundsException | IllegalArgumentException e) {
                System.out.println("Test case index " + index + ": Error! " + e.getMessage());
            }
        }
    }
}