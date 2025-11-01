public class Lse18 {

    // The predefined array
    private static final int[] anArray = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    /**
     * Returns the value at the specified index of the predefined array.
     *
     * @param index The index of the element to return.
     * @return The element at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueAtIndex(int index) {
        // Check if the index is within the valid range
        if (index >= 0 && index < anArray.length) {
            return anArray[index];
        } else {
            // Return a special value to indicate an invalid index
            return -1;
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("Test Case 1 (index = 3): " + getValueAtIndex(3));
        System.out.println("Test Case 2 (index = 0): " + getValueAtIndex(0));
        System.out.println("Test Case 3 (index = 9): " + getValueAtIndex(9));
        System.out.println("Test Case 4 (index = 10): " + getValueAtIndex(10)); // Out of bounds
        System.out.println("Test Case 5 (index = -1): " + getValueAtIndex(-1)); // Out of bounds
    }
}