import java.util.Arrays;

public class Task21 {

    /**
     * Given an array of integers, removes the smallest value.
     * This function does not mutate the original array.
     *
     * @param numbers The input array of integers.
     * @return A new array with the first occurrence of the smallest value removed.
     *         Returns an empty array if the input is empty or null.
     */
    public static int[] removeSmallest(int[] numbers) {
        if (numbers == null || numbers.length <= 1) {
            return new int[0];
        }

        // Find the index of the first occurrence of the minimum value.
        int minIndex = 0;
        for (int i = 1; i < numbers.length; i++) {
            if (numbers[i] < numbers[minIndex]) {
                minIndex = i;
            }
        }

        // Create a new array and copy elements, skipping the one at minIndex.
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
        int[][] testCases = {
            {1, 2, 3, 4, 5},
            {5, 3, 2, 1, 4},
            {2, 2, 1, 2, 1},
            {1},
            {}
        };

        for (int[] testCase : testCases) {
            int[] result = removeSmallest(testCase);
            System.out.println(Arrays.toString(result));
        }
    }
}