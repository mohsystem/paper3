import java.util.*;

public class Task139 {
    public static double computeCircleArea(double radius) {
        if (!Double.isFinite(radius)) {
            throw new IllegalArgumentException("Radius must be a finite number.");
        }
        if (radius < 0.0) {
            throw new IllegalArgumentException("Radius must be non-negative.");
        }
        final double threshold = Math.sqrt(Double.MAX_VALUE / Math.PI);
        if (radius > threshold) {
            return Double.POSITIVE_INFINITY;
        }
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        double[] tests = new double[] {0.0, 1.0, 2.5, 1e154, -3.0};
        for (double r : tests) {
            try {
                double area = computeCircleArea(r);
                System.out.println("radius=" + r + " area=" + area);
            } catch (IllegalArgumentException ex) {
                System.out.println("radius=" + r + " error=" + ex.getMessage());
            }
        }
    }
}