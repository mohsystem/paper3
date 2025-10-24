import java.util.Arrays;

public class Task27 {
    public static int findOutlier(int[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Input array must have length >= 3");
        }
        int oddCount = 0;
        int limit = Math.min(3, arr.length);
        for (int i = 0; i < limit; i++) {
            if (isOdd(arr[i])) oddCount++;
        }
        boolean majorityOdd = oddCount >= 2;
        for (int v : arr) {
            if (isOdd(v) != majorityOdd) {
                return v;
            }
        }
        throw new IllegalArgumentException("No outlier found");
    }

    private static boolean isOdd(int x) {
        return (x & 1) != 0;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {2, 4, 0, 100, 4, 11, 2602, 36},
            {160, 3, 1719, 19, 11, 13, -21},
            {3, 5, 7, 9, 2, 11, 13},
            {2, 4, 6, 8, 10, 3, 12},
            {-2, -4, -6, -8, -11, -12}
        };
        for (int[] t : tests) {
            System.out.println(findOutlier(t));
        }
    }
}