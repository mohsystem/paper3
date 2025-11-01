import java.util.Arrays;

public class Lse26 {
    public static double getMonthlySales(double[] sales, int i) {
        if (sales == null || i < 0 || i >= sales.length) {
            return 0.0d;
        }
        double v = sales[i];
        if (Double.isNaN(v) || Double.isInfinite(v)) {
            return 0.0d;
        }
        return v;
    }

    public static double totalFirstQuarter(double[] sales) {
        double sum = 0.0d;
        for (int i = 0; i < 3; i++) {
            sum += getMonthlySales(sales, i);
        }
        return sum;
    }

    public static void main(String[] args) {
        double[][] tests = new double[][]{
            new double[]{100.0, 200.0, 300.0},
            new double[]{50.5, 60.2, 70.3},
            new double[]{1000.0, 2000.0, 3000.0, 4000.0},
            new double[]{10.0},
            new double[]{-5.0, 15.0, 25.0}
        };

        for (int t = 0; t < tests.length; t++) {
            double res = totalFirstQuarter(tests[t]);
            System.out.printf("Test %d input=%s -> Q1 total=%.2f%n", t + 1, Arrays.toString(tests[t]), res);
        }
    }
}