import java.util.Arrays;

public class Task21 {
    public static int[] removeSmallest(int[] arr) {
        if (arr == null || arr.length == 0) return new int[0];
        int minVal = arr[0];
        int minIdx = 0;
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] < minVal) {
                minVal = arr[i];
                minIdx = i;
            }
        }
        int[] result = new int[arr.length - 1];
        int j = 0;
        for (int i = 0; i < arr.length; i++) {
            if (i != minIdx) {
                result[j++] = arr[i];
            }
        }
        return result;
    }

    private static void printArray(int[] a) {
        System.out.print("[");
        for (int i = 0; i < a.length; i++) {
            System.out.print(a[i]);
            if (i + 1 < a.length) System.out.print(",");
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{1,2,3,4,5},
            new int[]{5,3,2,1,4},
            new int[]{2,2,1,2,1},
            new int[]{},
            new int[]{7,7,7}
        };

        for (int[] test : tests) {
            int[] out = removeSmallest(test);
            printArray(out);
        }
    }
}