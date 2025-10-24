import java.lang.Math;

public class Task139 {

    /**
     * Calculates the area of a circle given its radius.
     * The area is calculated as PI * radius^2.
     *
     * @param radius The radius of the circle. Must be a non-negative number.
     * @return The area of the circle.
     * @throws IllegalArgumentException if the radius is negative.
     */
    public static double calculateCircleArea(double radius) {
        if (radius < 0) {
            throw new IllegalArgumentException("Radius cannot be negative.");
        }
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        double[] testRadii = {0.0, 1.0, 5.5, 10.0, -2.0};

        for (int i = 0; i < testRadii.length; i++) {
            try {
                double area = calculateCircleArea(testRadii[i]);
                System.out.printf("Test Case %d: Radius = %.2f, Area = %.4f%n", i + 1, testRadii[i], area);
            } catch (IllegalArgumentException e) {
                System.out.printf("Test Case %d: Radius = %.2f, Error: %s%n", i + 1, testRadii[i], e.getMessage());
            }
        }
    }
}