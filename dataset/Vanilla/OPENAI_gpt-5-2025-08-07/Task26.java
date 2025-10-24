import java.util.*;

public class Task26 {
    public static int findOdd(int[] arr) {
        int x = 0;
        for (int v : arr) x ^= v;
        return x;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {7},
            {0},
            {1, 1, 2},
            {0, 1, 0, 1, 0},
            {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println(findOdd(tests[i]));
        }
    }
}