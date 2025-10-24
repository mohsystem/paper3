import java.util.Arrays;

public class Task6 {
    private static final double EPS = 1e-9;

    private static boolean eq(double a, double b) {
        return Math.abs(a - b) <= EPS;
    }

    public static double findUniq(double[] a) {
        if (a == null || a.length < 3) {
            throw new IllegalArgumentException("Array must have at least 3 elements");
        }
        double majority;
        if (eq(a[0], a[1]) || eq(a[0], a[2])) {
            majority = a[0];
        } else {
            majority = a[1];
        }
        for (double v : a) {
            if (!eq(v, majority)) {
                return v;
            }
        }
        return Double.NaN;
    }

    public static void main(String[] args) {
        double[][] tests = new double[][] {
            {1, 1, 1, 2, 1, 1},
            {0, 0, 0.55, 0, 0},
            {-2, -2, -2, -3, -2},
            {5, 6, 5, 5, 5},
            {7.7, 7.7, 7.7, 7.7, 8.8}
        };

        for (double[] test : tests) {
            System.out.println(findUniq(test));
        }
    }
}