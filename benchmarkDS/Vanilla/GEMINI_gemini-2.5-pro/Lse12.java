public class Lse12 {
    /**
     * Returns the value from an array at a given index.
     *
     * @param arr The array of integers.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The integer value at the specified index.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Assuming the index is always within the valid bounds of the array.
        return arr[index];
    }

    public static void main(String[] args) {
        int[] sampleArray = {10, 20, 30, 40, 50};
        int arraySize = sampleArray.length;

        // 5 Test Cases
        System.out.println("Java Test Cases:");

        // Test Case 1
        System.out.println("Value at index 0: " + getValueFromArray(sampleArray, arraySize, 0));
        // Test Case 2
        System.out.println("Value at index 2: " + getValueFromArray(sampleArray, arraySize, 2));
        // Test Case 3
        System.out.println("Value at index 4: " + getValueFromArray(sampleArray, arraySize, 4));
        // Test Case 4
        System.out.println("Value at index 1: " + getValueFromArray(sampleArray, arraySize, 1));
        // Test Case 5
        System.out.println("Value at index 3: " + getValueFromArray(sampleArray, arraySize, 3));
    }
}