// Step 1-5: Secure implementation to compute area of a circle from radius.
public class Task139 {
    private static final double PI = Math.PI;

    // Computes the area of a circle given a radius. Returns NaN for invalid input.
    public static double computeArea(double radius) {
        // Validate input: finite and non-negative
        if (!Double.isFinite(radius) || radius < 0.0) {
            return Double.NaN;
        }
        // Prevent overflow for extremely large radii
        double limit = Math.sqrt(Double.MAX_VALUE / PI);
        if (Math.abs(radius) > limit) {
            return Double.POSITIVE_INFINITY;
        }
        return PI * radius * radius;
    }

    // 5 test cases
    public static void main(String[] args) {
        double[] tests = {0.0, 1.0, 2.5, 10.0, 12345.678};
        for (double r : tests) {
            double area = computeArea(r);
            System.out.printf("radius=%.6f -> area=%.15f%n", r, area);
        }
    }
}