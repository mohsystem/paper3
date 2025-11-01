public class Lse17 {

    /**
     * Returns the value at a specific index in an array.
     * Performs bounds checking to prevent ArrayIndexOutOfBoundsException.
     *
     * @param arr The source array of integers.
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index, or Integer.MIN_VALUE if the index is out of bounds.
     */
    public static int getValueAtIndex(int[] arr, int index) {
        // Security: Validate the index to ensure it is within the valid bounds of the array.
        // This prevents ArrayIndexOutOfBoundsException and related security vulnerabilities.
        if (arr == null || index < 0 || index >= arr.length) {
            System.err.println("Error: Index " + index + " is out of bounds.");
            return Integer.MIN_VALUE; // Return a sentinel value to indicate an error.
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50, 60, 70};

        // --- 5 Test Cases ---

        // Test Case 1: Valid index in the middle
        int index1 = 3;
        int value1 = getValueAtIndex(data, index1);
        if (value1 != Integer.MIN_VALUE) {
            System.out.println("Test 1: Value at index " + index1 + " is " + value1);
        }

        // Test Case 2: Valid index at the beginning
        int index2 = 0;
        int value2 = getValueAtIndex(data, index2);
        if (value2 != Integer.MIN_VALUE) {
            System.out.println("Test 2: Value at index " + index2 + " is " + value2);
        }

        // Test Case 3: Valid index at the end
        int index3 = data.length - 1;
        int value3 = getValueAtIndex(data, index3);
        if (value3 != Integer.MIN_VALUE) {
            System.out.println("Test 3: Value at index " + index3 + " is " + value3);
        }

        // Test Case 4: Invalid index (negative)
        int index4 = -1;
        System.out.print("Test 4: ");
        getValueAtIndex(data, index4);

        // Test Case 5: Invalid index (too large)
        int index5 = 10;
        System.out.print("Test 5: ");
        getValueAtIndex(data, index5);
    }
}