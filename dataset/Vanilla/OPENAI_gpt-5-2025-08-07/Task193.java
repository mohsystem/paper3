import java.util.*;

public class Task193 {
    public static boolean chalkboardXorGame(int[] nums) {
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
            {0},
            {5}
        };
        for (int[] t : tests) {
            System.out.println(chalkboardXorGame(t));
        }
    }
}