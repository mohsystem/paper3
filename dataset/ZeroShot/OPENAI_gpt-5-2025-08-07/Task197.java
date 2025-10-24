import java.security.SecureRandom;
import java.util.Arrays;

class Solution {
    private final double radius;
    private final double xCenter;
    private final double yCenter;
    private final SecureRandom rng;

    public Solution(double radius, double x_center, double y_center) {
        if (!(radius > 0.0) || Double.isNaN(radius) || Double.isInfinite(radius)) {
            throw new IllegalArgumentException("Radius must be positive and finite.");
        }
        if (Double.isNaN(x_center) || Double.isNaN(y_center) ||
            Double.isInfinite(x_center) || Double.isInfinite(y_center)) {
            throw new IllegalArgumentException("Center coordinates must be finite numbers.");
        }
        this.radius = radius;
        this.xCenter = x_center;
        this.yCenter = y_center;
        this.rng = new SecureRandom();
    }

    public double[] randPoint() {
        double u1 = rng.nextDouble(); // in [0,1)
        double u2 = rng.nextDouble(); // in [0,1)
        double r = Math.sqrt(u1) * radius;
        double theta = 2.0 * Math.PI * u2;
        double x = xCenter + r * Math.cos(theta);
        double y = yCenter + r * Math.sin(theta);
        return new double[]{x, y};
    }
}

public class Task197 {
    private static void printPoint(double[] p) {
        System.out.printf("[%.5f, %.5f]%n", p[0], p[1]);
    }

    public static void main(String[] args) {
        // 5 test cases (calls)
        Solution solution = new Solution(1.0, 0.0, 0.0);
        for (int i = 0; i < 5; i++) {
            double[] pt = solution.randPoint();
            printPoint(pt);
        }

        // Additional example tests
        Solution solution2 = new Solution(2.0, 3.0, -4.0);
        for (int i = 0; i < 5; i++) {
            double[] pt = solution2.randPoint();
            printPoint(pt);
        }
    }
}