public class Task139 {

    /**
     * Calculates the area of a circle.
     *
     * @param radius The radius of the circle. Must be a non-negative number.
     * @return The area of the circle, or -1.0 if the radius is negative.
     */
    public static double calculateCircleArea(double radius) {
        // Security: Ensure the radius is not negative, as it's physically impossible.
        if (radius < 0) {
            return -1.0; // Return an error indicator for invalid input.
        }
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        double[] testRadii = {10.0, 5.5, 0.0, 1.0, -7.0};

        System.out.println("Java Test Cases:");
        for (double radius : testRadii) {
            double area = calculateCircleArea(radius);
            if (area == -1.0) {
                System.out.println("Radius: " + radius + " -> Error: Radius cannot be negative.");
            } else {
                System.out.printf("Radius: %.2f -> Area: %.2f\n", radius, area);
            }
        }
    }
}