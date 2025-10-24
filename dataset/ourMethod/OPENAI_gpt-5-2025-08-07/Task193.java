import java.util.Arrays;

public class Task193 {
    // Returns true if Alice wins given optimal play; false otherwise.
    public static boolean aliceWins(int[] nums) {
        // Validate inputs per constraints
        if (nums == null || nums.length < 1 || nums.length > 1000) {
            return false; // fail closed on invalid input
        }
        int xor = 0;
        for (int v : nums) {
            if (v < 0 || v >= (1 << 16)) {
                return false; // fail closed on invalid value
            }
            xor ^= v;
        }
        // If xor is 0 at start, Alice wins. Else Alice wins iff length is even.
        return xor == 0 || (nums.length % 2 == 0);
    }

    private static void runTest(int[] nums) {
        boolean result = aliceWins(nums);
        System.out.println("nums=" + Arrays.toString(nums) + " -> " + result);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(new int[]{1, 1, 2});      // false
        runTest(new int[]{0, 1});         // true
        runTest(new int[]{1, 2, 3});      // true
        runTest(new int[]{5});            // false
        runTest(new int[]{1, 1, 1});      // false
    }
}