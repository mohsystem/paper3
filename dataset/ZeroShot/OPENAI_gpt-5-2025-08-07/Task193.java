import java.util.Arrays;

public class Task193 {
    public static boolean xorGame(int[] nums) {
        if (nums == null || nums.length == 0) {
            return true;
        }
        int x = 0;
        for (int v : nums) {
            x ^= v;
        }
        if (x == 0) {
            return true;
        }
        return (nums.length % 2 == 0);
    }

    private static void runTest(int[] arr) {
        System.out.println(xorGame(arr));
    }

    public static void main(String[] args) {
        runTest(new int[]{1, 1, 2});
        runTest(new int[]{0, 1});
        runTest(new int[]{1, 2, 3});
        runTest(new int[]{0});
        runTest(new int[]{1});
    }
}