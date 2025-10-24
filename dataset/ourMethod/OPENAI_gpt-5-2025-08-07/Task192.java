import java.util.Arrays;

public class Task192 {
    // Returns true if and only if Alice wins under optimal play.
    public static boolean aliceWins(int[] nums) {
        validateInput(nums);
        int xor = 0;
        for (int v : nums) {
            xor ^= v;
        }
        // Alice wins if initial XOR is 0 or array length is even.
        return xor == 0 || (nums.length % 2 == 0);
    }

    private static void validateInput(int[] nums) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        if (nums.length < 1 || nums.length > 1000) {
            throw new IllegalArgumentException("Array length must be in [1, 1000].");
        }
        for (int v : nums) {
            if (v < 0 || v >= (1 << 16)) {
                throw new IllegalArgumentException("Array elements must be in [0, 2^16). Found: " + v);
            }
        }
    }

    private static void runTest(int[] nums) {
        boolean result = aliceWins(nums);
        System.out.println("Input: " + Arrays.toString(nums) + " -> " + result);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(new int[]{1, 1, 2});   // false
        runTest(new int[]{0, 1});      // true
        runTest(new int[]{1, 2, 3});   // true
        runTest(new int[]{1});         // false
        runTest(new int[]{0});         // true
    }
}