import java.util.Arrays;

public class Lse17 {

    public static int getValueAtIndex(int[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Array reference is null.");
        }
        if (index < 0 || index >= arr.length) {
            throw new IndexOutOfBoundsException("Index out of bounds.");
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] arr = new int[] {10, 20, 30, 40, 50};
        int[] tests = new int[] {0, 2, 4, 5, -1};

        System.out.println("Array: " + Arrays.toString(arr));
        for (int idx : tests) {
            try {
                int value = getValueAtIndex(arr, idx);
                System.out.println("Index " + idx + " -> " + value);
            } catch (RuntimeException ex) {
                System.out.println("Index " + idx + " -> Error: " + ex.getMessage());
            }
        }
    }
}