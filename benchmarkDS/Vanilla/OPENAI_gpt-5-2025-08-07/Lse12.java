import java.util.*;

public class Lse12 {
    public static int getValueFromArray(int[] arr, int n, int idx) {
        return arr[idx];
    }

    public static void main(String[] args) {
        int[] a1 = new int[]{10, 20, 30, 40, 50};
        int[] a2 = new int[]{1, 2, 3, 4};
        int[] a3 = new int[]{7};
        int[] a4 = new int[]{100, 200};
        int[] a5 = new int[]{5, 4, 3, 2, 1, 0};
        int[][] tests = new int[][]{a1, a2, a3, a4, a5};

        Scanner sc = new Scanner(System.in);
        int idx = 0;
        if (sc.hasNextInt()) {
            idx = sc.nextInt();
        }

        int testIdx = Math.abs(idx) % tests.length;
        int[] selected = tests[testIdx];
        int n = selected.length;
        int safeIndex = Math.floorMod(idx, n);

        int result = getValueFromArray(selected, n, safeIndex);
        System.exit(result);
    }
}