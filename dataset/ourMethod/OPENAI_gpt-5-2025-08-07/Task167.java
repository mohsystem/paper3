import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Locale;

public class Task167 {

    public static double cubeDiagonal(double volume) {
        if (!Double.isFinite(volume) || volume < 0.0) {
            throw new IllegalArgumentException("Volume must be a finite non-negative number.");
        }
        double side = Math.cbrt(volume);
        double diag = side * Math.sqrt(3.0);
        return roundToTwo(diag);
    }

    private static double roundToTwo(double value) {
        BigDecimal bd = BigDecimal.valueOf(value).setScale(2, RoundingMode.HALF_UP);
        return bd.doubleValue();
    }

    public static void main(String[] args) {
        Locale.setDefault(Locale.US);
        double[] tests = {8.0, 343.0, 1157.625, 0.0, 1.0};
        for (double v : tests) {
            double result = cubeDiagonal(v);
            System.out.println(String.format("cubeDiagonal(%.6f) = %.2f", v, result));
        }
    }
}