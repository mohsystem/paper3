import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task9 {

    public static String oddOrEven(int[] nums) {
        if (nums == null || nums.length == 0) {
            return "even";
        }
        int parity = 0;
        for (int n : nums) {
            parity ^= Math.floorMod(n, 2);
        }
        return parity == 0 ? "even" : "odd";
    }

    private static void runTest(int[] arr) {
        System.out.println(oddOrEven(arr));
    }

    public static void main(String[] args) {
        runTest(new int[]{});               // even
        runTest(new int[]{0});              // even
        runTest(new int[]{0, 1, 4});        // odd
        runTest(new int[]{0, -1, -5});      // even
        runTest(new int[]{2, 3, 5, 7});     // odd
    }
}