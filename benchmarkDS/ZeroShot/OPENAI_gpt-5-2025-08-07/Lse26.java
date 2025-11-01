import java.util.Arrays;

public class Lse26 {
    public static double getMonthlySales(double[] sales, int i) {
        if (sales == null) {
            throw new IllegalArgumentException("Sales array must not be null.");
        }
        if (i < 0 || i >= sales.length) {
            throw new IndexOutOfBoundsException("Index out of bounds: " + i);
        }
        double val = sales[i];
        if (!Double.isFinite(val)) {
            throw new IllegalArgumentException("Sales value must be finite for index: " + i);
        }
        return val;
    }

    public static double computeFirstQuarterSales(double[] sales) {
        if (sales == null) {
            throw new IllegalArgumentException("Sales array must not be null.");
        }
        if (sales.length < 3) {
            throw new IllegalArgumentException("Sales array must contain at least 3 elements.");
        }
        double sum = 0.0;
        for (int i = 0; i <= 2; i++) {
            sum += getMonthlySales(sales, i);
        }
        return sum;
    }

    public static void main(String[] args) {
        double[][] testCases = new double[][]{
            {120.5, 340.75, 230.25},
            {100.0, 0.0, 300.0, 400.0},
            {0.0, 0.0, 0.0},
            {1e6, 2e6, 3e6},
            {500.0, -100.0, 50.0}
        };

        for (int t = 0; t < testCases.length; t++) {
            try {
                double result = computeFirstQuarterSales(testCases[t]);
                System.out.println("Test " + (t + 1) + " input=" + Arrays.toString(testCases[t]) + " => Q1 total=" + result);
            } catch (Exception ex) {
                System.out.println("Test " + (t + 1) + " input=" + Arrays.toString(testCases[t]) + " => Error: " + ex.getMessage());
            }
        }
    }
}