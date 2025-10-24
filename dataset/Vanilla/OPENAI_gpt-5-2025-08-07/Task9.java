import java.util.*;

public class Task9 {
    public static String oddOrEven(int[] arr) {
        if (arr == null || arr.length == 0) return "even";
        long parity = 0L;
        for (int x : arr) {
            parity ^= (Math.abs((long)x) & 1L);
        }
        return parity == 0L ? "even" : "odd";
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[0],
            new int[]{0},
            new int[]{0, 1, 4},
            new int[]{0, -1, -5},
            new int[]{1, 2, 3, 4, 5}
        };
        for (int[] t : tests) {
            System.out.println(oddOrEven(t));
        }
    }
}