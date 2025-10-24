// Chain-of-Through process:
// 1) Problem understanding: Given volume of a cube, compute main diagonal length: d = cbrt(V) * sqrt(3), rounded to two decimals.
// 2) Security requirements: Validate input is finite and non-negative; avoid undefined behavior.
// 3) Secure coding generation: Use precise math functions (Math.cbrt, Math.sqrt) and BigDecimal for rounding to two decimals with HALF_UP.
// 4) Code review: Checked for input validation, precision, and proper rounding.
// 5) Secure code output: Final code with validation and tests.

import java.math.BigDecimal;
import java.math.RoundingMode;

public class Task167 {
    public static double cubeDiagonal(double volume) {
        requireValidVolume(volume);
        double side = Math.cbrt(volume);
        double diagonal = side * Math.sqrt(3.0);
        return roundToTwo(diagonal);
    }

    private static double roundToTwo(double value) {
        BigDecimal bd = BigDecimal.valueOf(value);
        bd = bd.setScale(2, RoundingMode.HALF_UP);
        return bd.doubleValue();
    }

    private static void requireValidVolume(double volume) {
        if (!Double.isFinite(volume) || volume < 0.0) {
            throw new IllegalArgumentException("Volume must be a non-negative finite number.");
        }
    }

    public static void main(String[] args) {
        double[] tests = new double[] { 8.0, 343.0, 1157.625, 1.0, 0.0 };
        for (double v : tests) {
            try {
                double result = cubeDiagonal(v);
                System.out.printf("cubeDiagonal(%.6f) = %.2f%n", v, result);
            } catch (IllegalArgumentException ex) {
                System.out.printf("cubeDiagonal(%.6f) threw: %s%n", v, ex.getMessage());
            }
        }
    }
}