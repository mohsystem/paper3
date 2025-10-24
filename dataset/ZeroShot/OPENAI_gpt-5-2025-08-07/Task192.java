import java.util.Arrays;

public class Task192 {
    public static boolean xorGame(int[] nums) {
        if (nums == null) return false;
        int x = 0;
        for (int v : nums) {
            x ^= v;
        }
        if (x == 0) return true;
        return (nums.length % 2 == 0);
    }

    private static void runTest(int[] nums) {
        boolean res = xorGame(nums);
        System.out.println(Arrays.toString(nums) + " -> " + res);
    }

    public static void main(String[] args) {
        runTest(new int[]{1, 1, 2});
        runTest(new int[]{0, 1});
        runTest(new int[]{1, 2, 3});
        runTest(new int[]{5});
        runTest(new int[]{1, 2});
    }
}