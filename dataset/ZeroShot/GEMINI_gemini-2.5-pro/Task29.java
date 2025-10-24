import java.util.HashMap;
import java.util.Map;
import java.util.Arrays;

public class Task29 {

    /**
     * Finds two different items in the array that sum up to the target value.
     * This implementation uses a HashMap for O(n) time complexity.
     *
     * @param numbers An array of integers.
     * @param target  The target sum.
     * @return An array containing the indices of the two numbers.
     */
    public static int[] twoSum(int[] numbers, int target) {
        Map<Integer, Integer> numMap = new HashMap<>();
        for (int i = 0; i < numbers.length; i++) {
            int complement = target - numbers[i];
            if (numMap.containsKey(complement)) {
                return new int[]{numMap.get(complement), i};
            }
            numMap.put(numbers[i], i);
        }
        // According to the problem description, a solution always exists.
        // Throwing an exception is appropriate for an unreachable state.
        throw new IllegalArgumentException("No two sum solution found");
    }

    public static void main(String[] args) {
        // Test cases
        int[][] numbersCases = {
            {1, 2, 3},
            {1234, 5678, 9012},
            {2, 2, 3},
            {3, 2, 4},
            {2, 7, 11, 15}
        };
        int[] targetCases = {4, 14690, 4, 6, 9};

        for (int i = 0; i < numbersCases.length; i++) {
            int[] result = twoSum(numbersCases[i], targetCases[i]);
            // Sort for consistent output format
            Arrays.sort(result);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: numbers = " + Arrays.toString(numbersCases[i]) + ", target = " + targetCases[i]);
            System.out.println("Output: {" + result[0] + ", " + result[1] + "}");
            System.out.println();
        }
    }
}