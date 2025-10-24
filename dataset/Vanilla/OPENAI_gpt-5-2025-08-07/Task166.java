import java.util.*;

public class Task166 {
    public static int[] posNegSort(int[] arr) {
        if (arr == null) return null;
        int[] res = arr.clone();
        List<Integer> positives = new ArrayList<>();
        for (int x : arr) {
            if (x > 0) positives.add(x);
        }
        Collections.sort(positives);
        int idx = 0;
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] > 0) {
                res[i] = positives.get(idx++);
            } else {
                res[i] = arr[i];
            }
        }
        return res;
    }

    private static void printArray(int[] a) {
        System.out.println(Arrays.toString(a));
    }

    public static void main(String[] args) {
        int[][] tests = {
            new int[]{6, 3, -2, 5, -8, 2, -2},
            new int[]{6, 5, 4, -1, 3, 2, -1, 1},
            new int[]{-5, -5, -5, -5, 7, -5},
            new int[]{},
            new int[]{9, -3, 8, -1, -2, 7, 6}
        };
        for (int[] t : tests) {
            printArray(posNegSort(t));
        }
    }
}