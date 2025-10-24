import java.security.SecureRandom;
import java.util.Arrays;

public class Task197 {

    public static final class Solution {
        private final double radius;
        private final double xCenter;
        private final double yCenter;
        private final SecureRandom rng;
        private static final double TWO_PI = 2.0 * Math.PI;

        public Solution(double radius, double x_center, double y_center) {
            if (!Double.isFinite(radius) || !Double.isFinite(x_center) || !Double.isFinite(y_center)) {
                throw new IllegalArgumentException("Inputs must be finite numbers.");
            }
            if (radius <= 0.0) {
                throw new IllegalArgumentException("Radius must be > 0.");
            }
            this.radius = radius;
            this.xCenter = x_center;
            this.yCenter = y_center;
            this.rng = new SecureRandom();
        }

        public double[] randPoint() {
            double u = rng.nextDouble();
            double v = rng.nextDouble();
            double r = Math.sqrt(u) * radius;
            double theta = TWO_PI * v;
            double x = xCenter + r * Math.cos(theta);
            double y = yCenter + r * Math.sin(theta);
            return new double[]{x, y};
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        Solution[] tests = new Solution[]{
            new Solution(1.0, 0.0, 0.0),
            new Solution(2.5, 3.0, -4.0),
            new Solution(10.0, -100.0, 100.0),
            new Solution(0.5, 1e5, -1e5),
            new Solution(1e8, 1e7, -1e7)
        };

        for (int i = 0; i < tests.length; i++) {
            double[] p = tests[i].randPoint();
            System.out.println("Test " + (i + 1) + ": " + Arrays.toString(p));
        }
    }
}