import java.util.Arrays;

public class Task6 {
    public static double findUniq(double[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 numbers.");
        }

        double a = arr[0], b = arr[1], c = arr[2];
        double majority;
        if (Double.compare(a, b) == 0 || Double.compare(a, c) == 0) {
            majority = a;
        } else {
            majority = b; // since a != b and a != c, b == c is the majority
        }

        for (double v : arr) {
            if (Double.compare(v, majority) != 0) {
                return v;
            }
        }

        // Should not reach here if input satisfies problem constraints
        throw new IllegalArgumentException("No unique element found.");
    }

    public static void main(String[] args) {
        double[][] tests = new double[][]{
            new double[]{1, 1, 1, 2, 1, 1},
            new double[]{0, 0, 0.55, 0, 0},
            new double[]{3, 4, 4, 4, 4},
            new double[]{-1, -1, -1, -2, -1, -1},
            new double[]{Double.NaN, Double.NaN, 7.0, Double.NaN}
        };

        for (double[] test : tests) {
            System.out.println(findUniq(test));
        }
    }
}