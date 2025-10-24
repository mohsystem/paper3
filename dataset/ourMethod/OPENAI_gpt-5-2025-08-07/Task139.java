import java.util.Locale;

public class Task139 {
    private static final double MAX_SAFE_RADIUS = Math.sqrt(Double.MAX_VALUE / Math.PI);

    public static double computeCircleArea(double radius) {
        if (!Double.isFinite(radius)) {
            throw new IllegalArgumentException("Radius must be a finite number.");
        }
        if (radius < 0.0) {
            throw new IllegalArgumentException("Radius must be non-negative.");
        }
        if (radius > MAX_SAFE_RADIUS) {
            throw new IllegalArgumentException("Radius too large; area would overflow.");
        }
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        Locale.setDefault(Locale.US);
        double[] tests = {0.0, 1.0, 2.5, 1e150, -3.0};
        for (double r : tests) {
            try {
                double area = computeCircleArea(r);
                System.out.printf("radius=%.6f -> area=%.6f%n", r, area);
            } catch (IllegalArgumentException ex) {
                System.out.printf("radius=%.6f -> error=%s%n", r, ex.getMessage());
            }
        }
    }
}