public class Lse16 {

    /**
     * Returns the value at a given index in an integer array.
     * @param arr The input array.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueAtIndex(int[] arr, int index) {
        if (index >= 0 && index < arr.length) {
            return arr[index];
        } else {
            // Return a sentinel value to indicate the index is out of bounds
            return -1;
        }
    }

    public static void main(String[] args) {
        int[] data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

        // 5 test cases
        System.out.println("Test Case 1 (index 0): " + getValueAtIndex(data, 0));
        System.out.println("Test Case 2 (index 5): " + getValueAtIndex(data, 5));
        System.out.println("Test Case 3 (index 9): " + getValueAtIndex(data, 9));
        System.out.println("Test Case 4 (index -1): " + getValueAtIndex(data, -1));
        System.out.println("Test Case 5 (index 10): " + getValueAtIndex(data, 10));
    }
}