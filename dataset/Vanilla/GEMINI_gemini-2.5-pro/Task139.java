public class Task139 {

    /**
     * Calculates the area of a circle.
     *
     * @param radius The radius of the circle.
     * @return The area of the circle.
     */
    public static double calculateArea(double radius) {
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        double[] testRadii = {1.0, 5.0, 10.0, 0.0, 7.5};
        System.out.println("Java Results:");
        for (double radius : testRadii) {
            System.out.println("Radius: " + radius + ", Area: " + calculateArea(radius));
        }
    }
}