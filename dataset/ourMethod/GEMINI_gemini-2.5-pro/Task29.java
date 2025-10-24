import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task29 {

    /**
     * Finds two different items in the array that sum up to the target value.
     *
     * @param numbers The input array of integers. Must have at least 2 elements.
     * @param target The target integer value.
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
        // Per problem description, a valid solution always exists.
        // Returning an empty array or throwing an exception would be for a more general case.
        return new int[0];
    }

    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {1, 2, 3};
        int target1 = 4;
        int[] result1 = twoSum(nums1, target1);
        System.out.println("Test Case 1: " + Arrays.toString(result1)); // Expected: [0, 2] or [2, 0]

        // Test case 2
        int[] nums2 = {1234, 5678, 9012};
        int target2 = 14690;
        int[] result2 = twoSum(nums2, target2);
        System.out.println("Test Case 2: " + Arrays.toString(result2)); // Expected: [1, 2] or [2, 1]

        // Test case 3
        int[] nums3 = {2, 2, 3};
        int target3 = 4;
        int[] result3 = twoSum(nums3, target3);
        System.out.println("Test Case 3: " + Arrays.toString(result3)); // Expected: [0, 1] or [1, 0]
        
        // Test case 4
        int[] nums4 = {3, 2, 4};
        int target4 = 6;
        int[] result4 = twoSum(nums4, target4);
        System.out.println("Test Case 4: " + Arrays.toString(result4)); // Expected: [1, 2] or [2, 1]

        // Test case 5
        int[] nums5 = {3, 3};
        int target5 = 6;
        int[] result5 = twoSum(nums5, target5);
        System.out.println("Test Case 5: " + Arrays.toString(result5)); // Expected: [0, 1] or [1, 0]
    }
}