import java.math.BigDecimal;
import java.math.RoundingMode;

public class Task167 {
    public static double cubeDiagonal(double volume) {
        if (!Double.isFinite(volume) || volume < 0.0) {
            return Double.NaN;
        }
        double diag = Math.cbrt(volume) * Math.sqrt(3.0);
        return BigDecimal.valueOf(diag).setScale(2, RoundingMode.HALF_UP).doubleValue();
    }

    public static void main(String[] args) {
        double[] tests = {8.0, 343.0, 1157.625, 0.0, -5.0};
        for (double v : tests) {
            double res = cubeDiagonal(v);
            String out = Double.isNaN(res) ? "NaN" : String.format(java.util.Locale.ROOT, "%.2f", res);
            System.out.println("cubeDiagonal(" + v + ") = " + out);
        }
    }
}