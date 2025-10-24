import java.util.*;

public class Task167 {
    public static double cubeDiagonal(double volume) {
        double d = Math.cbrt(volume) * Math.sqrt(3.0);
        return Math.round(d * 100.0) / 100.0;
    }

    public static void main(String[] args) {
        double[] tests = {8, 343, 1157.625, 1, 0.125};
        for (double v : tests) {
            double res = cubeDiagonal(v);
            System.out.printf("%.2f%n", res);
        }
    }
}