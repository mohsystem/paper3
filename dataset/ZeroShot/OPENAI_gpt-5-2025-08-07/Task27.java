import java.util.Arrays;

public class Task27 {

    public static long findOutlier(long[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Input array must have length >= 3");
        }
        int evenCount = 0;
        for (int i = 0; i < 3; i++) {
            if ((arr[i] & 1L) == 0L) {
                evenCount++;
            }
        }
        boolean majorityEven = evenCount >= 2;
        for (long v : arr) {
            boolean isEven = (v & 1L) == 0L;
            if (isEven != majorityEven) {
                return v;
            }
        }
        throw new IllegalStateException("No outlier found");
    }

    public static void main(String[] args) {
        long[][] tests = new long[][]{
            new long[]{2, 4, 0, 100, 4, 11, 2602, 36},
            new long[]{160, 3, 1719, 19, 11, 13, -21},
            new long[]{-2, -4, -6, -8, -10, -3, -12},
            new long[]{1, 3, 5, 7, 8, 9, 11, 13},
            new long[]{42, 6, 8, 10, 12, 14, 16, 7}
        };

        for (long[] test : tests) {
            try {
                long outlier = findOutlier(test);
                System.out.println(outlier);
            } catch (RuntimeException e) {
                System.out.println("Error: " + e.getMessage() + " for input " + Arrays.toString(test));
            }
        }
    }
}