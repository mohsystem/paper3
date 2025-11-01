import java.util.Arrays;

public class Lse17 {

    // Returns the value at the specified index in the array after validating inputs.
    // Throws IllegalArgumentException on invalid inputs.
    public static int valueAtIndex(int[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        if (index < 0 || index >= arr.length) {
            throw new IllegalArgumentException("Index out of range");
        }
        return arr[index];
    }

    // 5 test cases
    public static void main(String[] args) {
        int[] data = new int[] {10, 20, 30, 40, 50};
        int[] testIndices = new int[] {0, 2, 4, -1, 5};

        System.out.println("Array: " + Arrays.toString(data));
        for (int i = 0; i < testIndices.length; i++) {
            int idx = testIndices[i];
            try {
                int value = valueAtIndex(data, idx);
                System.out.println("Test " + (i + 1) + ": index=" + idx + " -> value=" + value);
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i + 1) + ": index=" + idx + " -> error");
            }
        }
    }
}