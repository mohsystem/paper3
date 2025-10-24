import java.util.Arrays;

public class Task18 {
    public static int binaryArrayToNumber(int[] arr) {
        int res = 0;
        for (int bit : arr) {
            res = (res << 1) | (bit & 1);
        }
        return res;
    }

    private static void runTest(int[] arr) {
        int result = binaryArrayToNumber(arr);
        System.out.println(Arrays.toString(arr) + " => " + result);
    }

    public static void main(String[] args) {
        runTest(new int[]{0, 0, 0, 1});
        runTest(new int[]{0, 0, 1, 0});
        runTest(new int[]{0, 1, 0, 1});
        runTest(new int[]{1, 0, 0, 1});
        runTest(new int[]{1, 1, 1, 1});
    }
}