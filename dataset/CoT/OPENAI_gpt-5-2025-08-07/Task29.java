// Task29 - Java Implementation
// Chain-of-Through process:
// 1) Problem understanding: twoSum finds two distinct indices whose values sum to target.
// 2) Security requirements: avoid mutable shared state, validate inputs where reasonable, handle unexpected states safely.
// 3) Secure coding generation: use HashMap for O(n) lookups, no external I/O dependencies.
// 4) Code review: bounds-safe loops, no exposure of internal references, handle no-solution via exception.
// 5) Secure code output: final hardened code below.

import java.util.*;

public class Task29 {
    // Function: accepts input as parameters and returns output
    public static int[] twoSum(int[] nums, int target) {
        if (nums == null || nums.length < 2) {
            throw new IllegalArgumentException("Input array must have length >= 2.");
        }
        // Map value -> index
        Map<Integer, Integer> seen = new HashMap<>(Math.max(16, nums.length * 2));
        for (int i = 0; i < nums.length; i++) {
            int complement = target - nums[i];
            Integer idx = seen.get(complement);
            if (idx != null && idx != i) {
                return new int[]{idx, i};
            }
            // Store/overwrite index for current value
            seen.put(nums[i], i);
        }
        // As per problem constraints, input always has a valid solution
        throw new IllegalStateException("No valid two sum solution found.");
    }

    private static void printResult(int[] res) {
        System.out.println("[" + res[0] + ", " + res[1] + "]");
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        int[] r1 = twoSum(new int[]{1, 2, 3}, 4);    // Expected any valid pair, e.g., [0,2]
        int[] r2 = twoSum(new int[]{3, 2, 4}, 6);    // Expected [1,2]
        int[] r3 = twoSum(new int[]{2, 7, 11, 15}, 9); // Expected [0,1]
        int[] r4 = twoSum(new int[]{0, 4, 3, 0}, 0); // Expected [0,3]
        int[] r5 = twoSum(new int[]{-3, 4, 3, 90}, 0); // Expected [0,2]

        printResult(r1);
        printResult(r2);
        printResult(r3);
        printResult(r4);
        printResult(r5);
    }
}