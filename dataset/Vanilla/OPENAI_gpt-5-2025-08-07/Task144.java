import java.util.Arrays;

public class Task144 {
    public static int[] mergeSortedArrays(int[] a, int[] b) {
        if (a == null) a = new int[0];
        if (b == null) b = new int[0];
        int n = a.length, m = b.length;
        int[] res = new int[n + m];
        int i = 0, j = 0, k = 0;
        while (i < n && j < m) {
            if (a[i] <= b[j]) {
                res[k++] = a[i++];
            } else {
                res[k++] = b[j++];
            }
        }
        while (i < n) res[k++] = a[i++];
        while (j < m) res[k++] = b[j++];
        return res;
    }

    private static String arrayToString(int[] arr) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) sb.append(", ");
            sb.append(arr[i]);
        }
        sb.append("]");
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] testsA = {
            {1, 3, 5},
            {},
            {1, 1, 2, 2},
            {-5, -3, 0, 2},
            {1, 2, 7, 8, 9}
        };
        int[][] testsB = {
            {2, 4, 6},
            {1, 2, 3},
            {1, 2, 2, 3},
            {-4, -1, 3},
            {3, 4}
        };

        for (int t = 0; t < 5; t++) {
            int[] merged = mergeSortedArrays(testsA[t], testsB[t]);
            System.out.println(arrayToString(merged));
        }
    }
}