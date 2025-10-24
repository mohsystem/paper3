import java.util.Arrays;

public final class Task6 {
    private Task6() {
        // Prevent instantiation
    }

    public static double findUniq(double[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 numbers.");
        }
        double a = arr[0], b = arr[1], c = arr[2];
        double common = (a == b || a == c) ? a : b;
        for (double v : arr) {
            if (v != common) {
                return v;
            }
        }
        throw new IllegalStateException("No unique value found.");
    }

    public static void main(String[] args) {
        double[][] tests = new double[][]{
            new double[]{1, 1, 1, 2, 1, 1},
            new double[]{0, 0, 0.55, 0, 0},
            new double[]{3, 3, 4, 3, 3, 3},
            new double[]{9, 8, 8, 8, 8, 8},
            new double[]{-1, -1, -1, -2, -1, -1}
        };

        for (double[] t : tests) {
            double result = findUniq(t);
            System.out.println(result);
        }
    }
}