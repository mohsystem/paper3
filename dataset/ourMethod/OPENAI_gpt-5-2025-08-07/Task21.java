import java.util.Arrays;

public class Task21 {

    public static int[] removeSmallest(int[] arr) {
        if (arr == null || arr.length == 0) {
            return new int[0];
        }
        int minVal = arr[0];
        int minIdx = 0;
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] < minVal) {
                minVal = arr[i];
                minIdx = i;
            }
        }
        int[] result = new int[arr.length - 1];
        int k = 0;
        for (int i = 0; i < arr.length; i++) {
            if (i != minIdx) {
                result[k++] = arr[i];
            }
        }
        return result;
    }

    private static String formatArray(int[] a) {
        if (a == null || a.length == 0) return "[]";
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < a.length; i++) {
            if (i > 0) sb.append(',');
            sb.append(a[i]);
        }
        sb.append(']');
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] tests = new int[][] {
            new int[] {1,2,3,4,5},
            new int[] {5,3,2,1,4},
            new int[] {2,2,1,2,1},
            new int[] {},
            new int[] {-3,-1,-3,0}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] input = tests[i];
            int[] output = removeSmallest(input);
            System.out.println("Input:  " + formatArray(input));
            System.out.println("Output: " + formatArray(output));
            System.out.println("---");
        }
    }
}