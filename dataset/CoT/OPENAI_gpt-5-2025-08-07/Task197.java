import java.util.Arrays;
import java.util.concurrent.ThreadLocalRandom;

public class Task197 {
    // Chain-of-Through:
    // 1) Problem understanding: Sample uniformly within a circle by using r = R * sqrt(U), theta = 2*pi*V.
    // 2) Security requirements: Validate inputs (positive radius, finite values), avoid NaN/Inf.
    // 3) Secure coding generation: Use ThreadLocalRandom, avoid shared mutable state; immutable fields.
    // 4) Code review: Use sqrt(U) for area-uniform sampling, include circumference; handle edge cases.
    // 5) Secure code output: Provide clear API, no unsafe operations.

    static class Solution {
        private final double radius;
        private final double xCenter;
        private final double yCenter;

        public Solution(double radius, double x_center, double y_center) {
            if (!(radius > 0.0) || Double.isNaN(radius) || Double.isInfinite(radius)) {
                throw new IllegalArgumentException("Radius must be a positive finite number.");
            }
            if (!Double.isFinite(x_center) || !Double.isFinite(y_center)) {
                throw new IllegalArgumentException("Center coordinates must be finite numbers.");
            }
            this.radius = radius;
            this.xCenter = x_center;
            this.yCenter = y_center;
        }

        public double[] randPoint() {
            ThreadLocalRandom rng = ThreadLocalRandom.current();
            double u = rng.nextDouble(); // [0,1)
            double theta = rng.nextDouble(0.0, 2.0 * Math.PI); // [0,2π)
            double r = Math.sqrt(u) * this.radius;
            double x = this.xCenter + r * Math.cos(theta);
            double y = this.yCenter + r * Math.sin(theta);
            return new double[]{x, y};
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        Solution s1 = new Solution(1.0, 0.0, 0.0);
        Solution s2 = new Solution(3.5, 2.0, -1.0);
        Solution s3 = new Solution(10.0, 1000.0, -2000.0);
        Solution s4 = new Solution(0.0001, -0.5, 0.5);
        Solution s5 = new Solution(5.0, -100.0, 100.0);

        System.out.println(Arrays.toString(s1.randPoint()));
        System.out.println(Arrays.toString(s2.randPoint()));
        System.out.println(Arrays.toString(s3.randPoint()));
        System.out.println(Arrays.toString(s4.randPoint()));
        System.out.println(Arrays.toString(s5.randPoint()));
    }
}