import java.util.*;

public class Task166 {
    public static int[] posNegSort(int[] arr) {
        if (arr == null) return null;
        int[] res = Arrays.copyOf(arr, arr.length);
        List<Integer> positives = new ArrayList<>();
        for (int v : arr) {
            if (v > 0) positives.add(v);
        }
        Collections.sort(positives);
        int pi = 0;
        for (int i = 0; i < res.length; i++) {
            if (res[i] > 0) {
                res[i] = positives.get(pi++);
            }
        }
        return res;
    }

    private static void printArray(int[] a) {
        if (a == null) {
            System.out.println("null");
            return;
        }
        System.out.println(Arrays.toString(a));
    }

    public static void main(String[] args) {
        int[][] tests = new int[][] {
            new int[] {6, 3, -2, 5, -8, 2, -2},
            new int[] {6, 5, 4, -1, 3, 2, -1, 1},
            new int[] {-5, -5, -5, -5, 7, -5},
            new int[] {},
            new int[] {5, -1, -3, 2, 1}
        };
        for (int[] test : tests) {
            int[] out = posNegSort(test);
            printArray(out);
        }
    }
}