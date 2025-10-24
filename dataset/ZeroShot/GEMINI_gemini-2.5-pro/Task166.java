import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Arrays;

public class Task166 {

    /**
     * Sorts the positive numbers in an array in ascending order,
     * while keeping the negative numbers in their original positions.
     *
     * @param arr The input array of integers.
     * @return A new array with positive numbers sorted, or an empty array if input is null or empty.
     */
    public static int[] posNegSort(int[] arr) {
        if (arr == null) {
            return new int[0];
        }
        if (arr.length == 0) {
            return Arrays.copyOf(arr, 0);
        }

        // 1. Extract positive numbers into a separate list.
        List<Integer> positives = new ArrayList<>();
        for (int num : arr) {
            if (num > 0) {
                positives.add(num);
            }
        }

        // 2. Sort the list of positive numbers.
        Collections.sort(positives);

        // 3. Create a new result array and populate it.
        //    Iterate through the original array. If an element was positive,
        //    replace it with the next sorted positive number.
        //    Otherwise, keep the negative number.
        int[] result = new int[arr.length];
        int positiveIndex = 0;
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] > 0) {
                result[i] = positives.get(positiveIndex++);
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
            {5, -1, 4, -2, 3, -3, 2, -4, 1}
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original: " + Arrays.toString(testCases[i]));
            int[] result = posNegSort(testCases[i]);
            System.out.println("Sorted:   " + Arrays.toString(result));
            System.out.println();
        }
    }
}