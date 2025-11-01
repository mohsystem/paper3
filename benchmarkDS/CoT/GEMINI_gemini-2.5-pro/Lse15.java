public class Lse15 {

    // The predefined array
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Takes an index and returns the value at that index in the id_sequence array.
     * Performs bounds checking to prevent security vulnerabilities.
     *
     * @param index The index of the element to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueAtIndex(int index) {
        // Security check: ensure the index is within the valid bounds of the array.
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        } else {
            // Return a sentinel value to indicate an error (index out of bounds).
            return -1;
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid index in the middle
        int index1 = 3;
        System.out.println("Test Case 1: Index = " + index1 + ", Value = " + getValueAtIndex(index1));

        // Test Case 2: Valid index at the beginning
        int index2 = 0;
        System.out.println("Test Case 2: Index = " + index2 + ", Value = " + getValueAtIndex(index2));

        // Test Case 3: Valid index at the end
        int index3 = id_sequence.length - 1;
        System.out.println("Test Case 3: Index = " + index3 + ", Value = " + getValueAtIndex(index3));

        // Test Case 4: Invalid index (negative)
        int index4 = -1;
        System.out.println("Test Case 4: Index = " + index4 + ", Value = " + getValueAtIndex(index4));

        // Test Case 5: Invalid index (out of bounds)
        int index5 = id_sequence.length;
        System.out.println("Test Case 5: Index = " + index5 + ", Value = " + getValueAtIndex(index5));
    }
}