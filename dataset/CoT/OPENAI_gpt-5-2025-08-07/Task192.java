import java.util.Arrays;

public class Task192 {
    // Chain-of-Through Process:
    // 1. Problem understanding: Implement xorGame to decide if Alice wins with optimal play.
    // 2. Security requirements: Handle null/empty safely, avoid unchecked assumptions.
    // 3. Secure coding generation: Pure computation, no external input, no side effects.
    // 4. Code review: Simple XOR and parity logic, constant-time per element.
    // 5. Secure code output: Final validated logic.

    public static boolean xorGame(int[] nums) {
        if (nums == null || nums.length == 0) {
            // XOR of no elements is 0; starting player wins by rule.
            return true;
        }
        int x = 0;
        for (int v : nums) {
            // Defensive mask (though inputs are within 0..2^16-1 by constraints)
            x ^= (v & 0xFFFF);
        }
        if (x == 0) return true;
        return (nums.length % 2 == 0);
    }

    private static void runTest(int[] nums) {
        boolean res = xorGame(nums);
        System.out.println("nums=" + Arrays.toString(nums) + " -> " + res);
    }

    public static void main(String[] args) {
        runTest(new int[]{1, 1, 2});   // false
        runTest(new int[]{0, 1});      // true
        runTest(new int[]{1, 2, 3});   // true
        runTest(new int[]{1});         // false
        runTest(new int[]{1, 1});      // true
    }
}