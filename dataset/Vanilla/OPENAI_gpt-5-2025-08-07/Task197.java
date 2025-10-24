import java.util.Random;

public class Task197 {
    static class Solution {
        private final double radius;
        private final double x_center;
        private final double y_center;
        private final Random rng;

        public Solution(double radius, double x_center, double y_center) {
            this.radius = radius;
            this.x_center = x_center;
            this.y_center = y_center;
            this.rng = new Random();
        }

        public double[] randPoint() {
            double u = rng.nextDouble();
            double r = Math.sqrt(u) * radius;
            double theta = 2.0 * Math.PI * rng.nextDouble();
            double x = x_center + r * Math.cos(theta);
            double y = y_center + r * Math.sin(theta);
            return new double[] { x, y };
        }
    }

    private static void printPoint(double[] p) {
        System.out.println("[" + p[0] + ", " + p[1] + "]");
    }

    public static void main(String[] args) {
        Solution[] tests = new Solution[] {
            new Solution(1.0, 0.0, 0.0),
            new Solution(2.0, 1.0, 1.0),
            new Solution(0.5, -2.0, -3.0),
            new Solution(10.0, 5.0, -7.0),
            new Solution(3.0, 100.0, 200.0)
        };
        for (Solution s : tests) {
            double[] p = s.randPoint();
            printPoint(p);
        }
    }
}