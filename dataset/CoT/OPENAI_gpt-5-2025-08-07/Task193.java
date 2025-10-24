import java.util.Arrays;

public class Task193 {
    // Determine if Alice wins the XOR game
    public static boolean xorGame(int[] nums) {
        if (nums == null) throw new IllegalArgumentException("Input array cannot be null");
        int xor = 0;
        for (int v : nums) {
            xor ^= v;
        }
        if (xor == 0) return true;
        return nums.length % 2 == 0;
    }

    private static String boolStr(boolean b) {
        return b ? "true" : "false";
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 1, 2},   // false
            {0, 1},      // true
            {1, 2, 3},   // true
            {5},         // false
            {2, 2}       // true
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Input: " + Arrays.toString(tests[i]) + " -> " + boolStr(xorGame(tests[i])));
        }
    }
}