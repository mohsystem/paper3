import java.util.Arrays;

public class Task14 {

    public static int findEvenIndex(int[] arr) {
        long total = 0;
        for (int v : arr) total += v;
        long left = 0;
        for (int i = 0; i < arr.length; i++) {
            long right = total - left - arr[i];
            if (left == right) return i;
            left += arr[i];
        }
        return -1;
        }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 2, 3, 4, 3, 2, 1},
            {1, 100, 50, -51, 1, 1},
            {20, 10, -80, 10, 10, 15, 35},
            {1, 2, 3, 4, 5, 6},
            {0, 0, 0, 0}
        };
        for (int[] t : tests) {
            System.out.println(Arrays.toString(t) + " -> " + findEvenIndex(t));
        }
    }
}