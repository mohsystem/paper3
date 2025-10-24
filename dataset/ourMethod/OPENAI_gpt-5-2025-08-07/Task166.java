import java.util.Arrays;

public class Task166 {

    public static int[] posNegSort(int[] input) {
        if (input == null || input.length == 0) {
            return new int[0];
        }

        int n = input.length;
        int posCount = 0;
        for (int v : input) {
            if (v > 0) {
                posCount++;
            }
        }

        int[] positives = new int[posCount];
        int idx = 0;
        for (int v : input) {
            if (v > 0) {
                positives[idx++] = v;
            }
        }

        Arrays.sort(positives);

        int[] result = new int[n];
        int p = 0;
        for (int i = 0; i < n; i++) {
            int v = input[i];
            if (v < 0) {
                result[i] = v;
            } else {
                result[i] = positives[p++];
            }
        }

        return result;
    }

    private static String arrToString(int[] arr) {
        return Arrays.toString(arr == null ? new int[0] : arr);
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {6, 3, -2, 5, -8, 2, -2},
            {6, 5, 4, -1, 3, 2, -1, 1},
            {-5, -5, -5, -5, 7, -5},
            {},
            {-1, -3, 2, 1, 4}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] input = tests[i];
            int[] output = posNegSort(input);
            System.out.println("Test " + (i + 1) + " input:  " + arrToString(input));
            System.out.println("Test " + (i + 1) + " output: " + arrToString(output));
            System.out.println();
        }
    }
}