import java.util.*;

public class Lse17 {
    public static int getValueAtIndex(int[] arr, int index) {
        if (arr == null) throw new IllegalArgumentException("Array is null");
        if (index < 0 || index >= arr.length) throw new IndexOutOfBoundsException("Invalid index");
        return arr[index];
    }

    public static void main(String[] args) {
        int[] arr = {10, 25, 37, 48, 59};
        int[] testIndices = {0, 1, 2, 3, 4};

        for (int idx : testIndices) {
            int value = getValueAtIndex(arr, idx);
            System.out.println(value);
        }
    }
}