public class Lse18 {

    /**
     * Returns the value at the specified index in the array.
     * Performs bounds checking to prevent security vulnerabilities.
     *
     * @param arr The source array.
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     * @throws IndexOutOfBoundsException if the index is out of range (index < 0 || index >= arr.length).
     */
    public static int getValueAtIndex(int[] arr, int index) {
        // Security: Check if the index is within the valid bounds of the array.
        if (index < 0 || index >= arr.length) {
            // Throwing an exception is the standard way to handle out-of-bounds access in Java.
            throw new IndexOutOfBoundsException("Index " + index + " is out of bounds for array of size " + arr.length);
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        
        // 5 Test Cases
        int[] testIndexes = {3, 0, 9, -1, 10};

        for (int index : testIndexes) {
            try {
                int value = getValueAtIndex(data, index);
                System.out.println("Test Case - Index: " + index);
                System.out.println("Value at index " + index + " is: " + value);
            } catch (IndexOutOfBoundsException e) {
                System.out.println("Test Case - Index: " + index);
                System.out.println("Error: " + e.getMessage());
            }
            System.out.println("--------------------");
        }
    }
}