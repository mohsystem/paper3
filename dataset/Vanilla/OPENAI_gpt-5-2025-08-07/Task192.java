import java.util.*;

public class Task192 {
    public static boolean xorGame(int[] nums) {
        int xor = 0;
        for (int v : nums) xor ^= v;
        if (xor == 0) return true;
        return nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 1, 2},
            {0, 1},
            {1, 2, 3},
            {2},
            {1, 1}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println(xorGame(tests[i]));
        }
    }
}