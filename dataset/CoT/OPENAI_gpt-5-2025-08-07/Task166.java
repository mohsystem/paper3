import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Task166 {
    // Sort positive numbers ascending, keep negatives in original positions.
    public static int[] posNegSort(int[] arr) {
        if (arr == null) return new int[0];
        int n = arr.length;
        int[] result = new int[n];
        List<Integer> positives = new ArrayList<>();
        for (int v : arr) {
            if (v > 0) positives.add(v);
        }
        Collections.sort(positives);
        int idx = 0;
        for (int i = 0; i < n; i++) {
            if (arr[i] < 0) {
                result[i] = arr[i];
            } else {
                result[i] = positives.get(idx++);
            }
        }
        return result;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {6, 3, -2, 5, -8, 2, -2},
            {6, 5, 4, -1, 3, 2, -1, 1},
            {-5, -5, -5, -5, 7, -5},
            {},
            {5, -1, -2, 4, 3}
        };
        for (int[] t : tests) {
            System.out.println(Arrays.toString(posNegSort(t)));
        }
    }
}