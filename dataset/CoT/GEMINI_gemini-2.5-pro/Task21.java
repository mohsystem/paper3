import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task21 {

    /**
     * Given an array of integers, remove the smallest value.
     * Do not mutate the original array.
     * If there are multiple elements with the same value, remove the one with the lowest index.
     * If you get an empty array, return an empty array.
     *
     * @param numbers The input array of integers.
     * @return A new array with the smallest value removed.
     */
    public static int[] removeSmallest(int[] numbers) {
        if (numbers == null || numbers.length == 0) {
            return new int[0];
        }

        int minIndex = 0;
        int minValue = numbers[0];
        for (int i = 1; i < numbers.length; i++) {
            if (numbers[i] < minValue) {
                minValue = numbers[i];
                minIndex = i;
            }
        }

        int[] result = new int[numbers.length - 1];
        int resultIndex = 0;
        for (int i = 0; i < numbers.length; i++) {
            if (i == minIndex) {
                continue;
            }
            result[resultIndex++] = numbers[i];
        }

        return result;
    }

    public static void main(String[] args) {
        // Test Cases
        int[][] testCases = {
            {1, 2, 3, 4, 5},
            {5, 3, 2, 1, 4},
            {2, 2, 1, 2, 1},
            {},
            {3, 3, 3, 3, 3}
        };

        for (int i = 0; i < testCases.length; i++) {
            int[] original = testCases[i];
            int[] result = removeSmallest(original);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original: " + Arrays.toString(original));
            System.out.println("Result:   " + Arrays.toString(result));
            System.out.println();
        }
    }
}