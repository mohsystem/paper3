import java.util.Arrays;

public class Task9 {
    public static String oddOrEven(int[] arr) {
        if (arr == null || arr.length == 0) {
            return "even";
        }
        int parity = 0;
        for (int x : arr) {
            parity ^= (x & 1);
        }
        return parity == 0 ? "even" : "odd";
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{0},
            new int[]{0, 1, 4},
            new int[]{0, -1, -5},
            new int[]{},
            new int[]{2, 2, 2, 3}
        };
        for (int[] t : tests) {
            System.out.println(Arrays.toString(t) + " -> " + oddOrEven(t));
        }
    }
}