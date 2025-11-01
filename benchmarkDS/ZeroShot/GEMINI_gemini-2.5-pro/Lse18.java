import java.util.Arrays;

public class Lse18 {

    /**
     * Securely gets the value at a specified index of an integer array.
     *
     * @param arr The array to access.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     * @throws IllegalArgumentException if the array is null.
     * @throws IndexOutOfBoundsException if the index is out of bounds (index < 0 || index >= arr.length).
     */
    public static int getValueAtIndex(int[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        // Secure: Check if the index is within the valid range.
        if (index < 0 || index >= arr.length) {
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array of length " + arr.length);
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        System.out.println("Array: " + Arrays.toString(data));

        // Test Cases
        int[] testIndexes = {5, 0, 9, -1, 10};

        for (int index : testIndexes) {
            try {
                int value = getValueAtIndex(data, index);
                System.out.println("Test case index: " + index + " -> Success! Value: " + value);
            } catch (IndexOutOfBoundsException | IllegalArgumentException e) {
                System.out.println("Test case index: " + index + " -> Caught expected exception: " + e.getMessage());
            }
        }
    }
}