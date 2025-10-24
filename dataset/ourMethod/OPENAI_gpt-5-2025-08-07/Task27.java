import java.util.Arrays;

public class Task27 {
    public static int findOutlier(final int[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Input array must have length >= 3.");
        }

        int evenCount = 0;
        for (int i = 0; i < 3; i++) {
            if ((arr[i] % 2) == 0) {
                evenCount++;
            }
        }
        boolean majorityEven = evenCount >= 2;

        int outlier = 0;
        int mismatchCount = 0;
        for (int val : arr) {
            boolean isEven = (val % 2) == 0;
            if (isEven != majorityEven) {
                mismatchCount++;
                outlier = val;
                if (mismatchCount > 1) {
                    break;
                }
            }
        }

        if (mismatchCount != 1) {
            throw new IllegalArgumentException("Array does not contain exactly one parity outlier.");
        }

        return outlier;
    }

    private static void runTest(int[] input, int expected) {
        try {
            int result = findOutlier(input);
            System.out.println("Input: " + Arrays.toString(input) + " -> Outlier: " + result + " | Expected: " + expected);
        } catch (IllegalArgumentException ex) {
            System.out.println("Input: " + Arrays.toString(input) + " -> Error: " + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        runTest(new int[]{2, 4, 0, 100, 4, 11, 2602, 36}, 11);
        runTest(new int[]{160, 3, 1719, 19, 11, 13, -21}, 160);
        runTest(new int[]{1, 1, 2}, 2);
        runTest(new int[]{2, -6, 8, -10, -3}, -3);
        runTest(new int[]{3, 5, 7, 9, 2}, 2);
    }
}