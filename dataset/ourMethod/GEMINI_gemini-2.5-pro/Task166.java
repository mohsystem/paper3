import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Task166 {

    /**
     * Sorts the positive numbers in an array in ascending order,
     * while keeping the negative numbers in their original positions.
     *
     * @param arr The input array of integers.
     * @return A new array with positive numbers sorted.
     */
    public static int[] posNegSort(int[] arr) {
        if (arr == null || arr.length == 0) {
            return new int[0];
        }

        List<Integer> positives = new ArrayList<>();
        for (int num : arr) {
            if (num > 0) {
                positives.add(num);
            }
        }

        Collections.sort(positives);

        int[] result = new int[arr.length];
        int posIndex = 0;
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] > 0) {
                result[i] = positives.get(posIndex++);
            } else {
                result[i] = arr[i];
            }
        }
        return result;
    }

    public static void main(String[] args) {
        int[][] testCases = {
            {6, 3, -2, 5, -8, 2, -2},
            {6, 5, 4, -1, 3, 2, -1, 1},
            {-5, -5, -5, -5, 7, -5},
            {},
            {1, -1, 2, -2, 3, -3}
        };

        for (int[] testCase : testCases) {
            System.out.println("Original: " + Arrays.toString(testCase));
            int[] sorted = posNegSort(testCase);
            System.out.println("Sorted:   " + Arrays.toString(sorted));
            System.out.println();
        }
    }
}