import java.util.Arrays;

public class Task29 {
    /**
     * Finds two indices of numbers in an array that add up to a target value.
     *
     * @param numbers The input array of integers.
     * @param target  The target sum.
     * @return An array containing the two indices. Returns an empty array if no solution is found.
     */
    public static int[] twoSum(int[] numbers, int target) {
        for (int i = 0; i < numbers.length; i++) {
            for (int j = i + 1; j < numbers.length; j++) {
                if (numbers[i] + numbers[j] == target) {
                    return new int[]{i, j};
                }
            }
        }
        // As per problem description, a solution always exists.
        // This is to satisfy the compiler for a return path.
        return new int[0];
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] nums1 = {1, 2, 3};
        int target1 = 4;
        System.out.println("Test Case 1: " + Arrays.toString(twoSum(nums1, target1))); // Expected: [0, 2]

        // Test Case 2
        int[] nums2 = {1234, 5678, 9012};
        int target2 = 14690;
        System.out.println("Test Case 2: " + Arrays.toString(twoSum(nums2, target2))); // Expected: [1, 2]

        // Test Case 3
        int[] nums3 = {2, 2, 3};
        int target3 = 4;
        System.out.println("Test Case 3: " + Arrays.toString(twoSum(nums3, target3))); // Expected: [0, 1]

        // Test Case 4
        int[] nums4 = {3, 2, 4};
        int target4 = 6;
        System.out.println("Test Case 4: " + Arrays.toString(twoSum(nums4, target4))); // Expected: [1, 2]

        // Test Case 5
        int[] nums5 = {2, 7, 11, 15};
        int target5 = 9;
        System.out.println("Test Case 5: " + Arrays.toString(twoSum(nums5, target5))); // Expected: [0, 1]
    }
}