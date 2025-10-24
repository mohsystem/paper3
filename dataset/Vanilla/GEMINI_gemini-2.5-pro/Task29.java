import java.util.HashMap;
import java.util.Map;
import java.util.Arrays;

public class Task29 {
    /**
     * Finds two indices of numbers in an array that add up to a target value.
     *
     * @param numbers The input array of integers.
     * @param target The target sum.
     * @return An array of two integers representing the indices of the two numbers.
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
        // According to the problem description, a solution always exists,
        // so this part should not be reached.
        throw new IllegalArgumentException("No two sum solution");
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] result1 = twoSum(new int[]{1, 2, 3}, 4);
        System.out.println("Test 1: " + Arrays.toString(result1)); // Expected: [0, 2]

        // Test Case 2
        int[] result2 = twoSum(new int[]{1234, 5678, 9012}, 14690);
        System.out.println("Test 2: " + Arrays.toString(result2)); // Expected: [1, 2]

        // Test Case 3
        int[] result3 = twoSum(new int[]{2, 2, 3}, 4);
        System.out.println("Test 3: " + Arrays.toString(result3)); // Expected: [0, 1]

        // Test Case 4
        int[] result4 = twoSum(new int[]{3, 2, 4}, 6);
        System.out.println("Test 4: " + Arrays.toString(result4)); // Expected: [1, 2]
        
        // Test Case 5
        int[] result5 = twoSum(new int[]{2, 7, 11, 15}, 9);
        System.out.println("Test 5: " + Arrays.toString(result5)); // Expected: [0, 1]
    }
}