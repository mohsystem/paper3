import java.util.Arrays;

public class Task21 {

    /**
     * Given an array of integers, remove the smallest value.
     * Do not mutate the original array.
     * If there are multiple elements with the same value, remove the one with the lowest index.
     * If you get an empty array, return an empty array.
     *
     * @param numbers The input array of integers.
     * @return A new array with the smallest element removed.
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
            if (i != minIndex) {
                result[resultIndex++] = numbers[i];
            }
        }
        return result;
    }

    public static void main(String[] args) {
        // Test cases
        int[][] testCases = {
            {1, 2, 3, 4, 5},
            {5, 3, 2, 1, 4},
            {2, 2, 1, 2, 1},
            {10},
            {}
        };

        for (int[] testCase : testCases) {
            System.out.println("Input:  " + Arrays.toString(testCase));
            int[] originalCopy = Arrays.copyOf(testCase, testCase.length); // To check for mutation
            int[] result = removeSmallest(testCase);
            System.out.println("Output: " + Arrays.toString(result));
            System.out.println("Original untouched: " + Arrays.equals(testCase, originalCopy));
            System.out.println("---");
        }
    }
}