public class Lse12 {

    /**
     * Gets a value from an array at a specified index.
     * Securely checks if the index is within the valid bounds of the array.
     *
     * @param arr The source array of integers.
     * @param size The size of the array.
     * @param index The index from which to retrieve the value.
     * @return The value at the given index, or -1 if the index is out of bounds.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Secure: Validate that the array is not null and the index is within bounds.
        // This prevents NullPointerException and ArrayIndexOutOfBoundsException.
        if (arr == null || index < 0 || index >= size) {
            System.err.println("Error: Index " + index + " is out of bounds for array of size " + size + ".");
            return -1; // Return a designated error code.
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50};
        int size = data.length;

        System.out.println("Running 5 test cases...");

        // Test Case 1: Valid index in the middle
        int index1 = 2;
        System.out.println("\nTest Case 1: index = " + index1);
        int value1 = getValueFromArray(data, size, index1);
        System.out.println("Value returned: " + value1); // Expected: 30
        
        // Test Case 2: Valid index at the beginning
        int index2 = 0;
        System.out.println("\nTest Case 2: index = " + index2);
        int value2 = getValueFromArray(data, size, index2);
        System.out.println("Value returned: " + value2); // Expected: 10
        
        // Test Case 3: Invalid negative index
        int index3 = -1;
        System.out.println("\nTest Case 3: index = " + index3);
        int value3 = getValueFromArray(data, size, index3);
        System.out.println("Value returned: " + value3); // Expected: -1
        
        // Test Case 4: Invalid index equal to size
        int index4 = size;
        System.out.println("\nTest Case 4: index = " + index4);
        int value4 = getValueFromArray(data, size, index4);
        System.out.println("Value returned: " + value4); // Expected: -1
        
        // Test Case 5: Valid index at the end
        int index5 = 4;
        System.out.println("\nTest Case 5: index = " + index5);
        int value5 = getValueFromArray(data, size, index5);
        System.out.println("Value returned: " + value5); // Expected: 50
    }
}