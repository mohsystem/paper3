import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task29 {

    public static int[] twoSum(final int[] numbers, final int target) {
        if (numbers == null || numbers.length < 2) {
            throw new IllegalArgumentException("Input array must have length >= 2.");
        }
        final Map<Integer, Integer> indexByValue = new HashMap<>();
        for (int i = 0; i < numbers.length; i++) {
            int val = numbers[i];
            int complement = target - val;
            Integer idx = indexByValue.get(complement);
            if (idx != null) {
                return new int[]{idx, i};
            }
            // Store first occurrence to ensure different indices
            if (!indexByValue.containsKey(val)) {
                indexByValue.put(val, i);
            }
        }
        throw new IllegalArgumentException("No two sum solution found.");
    }

    private static void runTest(int[] nums, int target) {
        try {
            int[] res = twoSum(nums, target);
            System.out.println("Input: " + Arrays.toString(nums) + ", target=" + target + " -> " + Arrays.toString(res));
        } catch (IllegalArgumentException ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        runTest(new int[]{1, 2, 3}, 4);           // expected [0,2]
        runTest(new int[]{3, 2, 4}, 6);           // expected [1,2]
        runTest(new int[]{3, 3}, 6);              // expected [0,1]
        runTest(new int[]{-1, -2, -3, -4, -5}, -8); // expected [2,4]
        runTest(new int[]{0, 4, 3, 0}, 0);        // expected [0,3]
    }
}