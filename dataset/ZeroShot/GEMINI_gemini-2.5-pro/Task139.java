import java.lang.Math;
import java.util.Locale;

public class Task139 {

    /**
     * Calculates the area of a circle securely.
     *
     * @param radius The radius of the circle. Must be a non-negative number.
     * @return The area of the circle.
     * @throws IllegalArgumentException if the radius is negative or not a finite number.
     */
    public static double calculateCircleArea(double radius) {
        if (radius < 0) {
            throw new IllegalArgumentException("Radius cannot be negative.");
        }
        if (Double.isInfinite(radius) || Double.isNaN(radius)) {
             throw new IllegalArgumentException("Radius must be a finite number.");
        }
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        // Set locale to ensure consistent decimal point formatting
        Locale.setDefault(Locale.US);
        
        double[] testRadii = {0.0, 1.0, 5.5, 10.0, -2.0};
        System.out.println("Running Java test cases:");

        for (double radius : testRadii) {
            try {
                double area = calculateCircleArea(radius);
                System.out.printf("Radius: %.2f, Area: %.2f\n", radius, area);
            } catch (IllegalArgumentException e) {
                System.out.printf("Radius: %.2f, Error: %s\n", radius, e.getMessage());
            }
        }
    }
}