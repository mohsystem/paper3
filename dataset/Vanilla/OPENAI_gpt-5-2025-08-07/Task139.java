public class Task139 {
    public static double computeArea(double radius) {
        return Math.PI * radius * radius;
    }

    public static void main(String[] args) {
        double[] radii = {0.0, 1.0, 2.5, 10.0, 123.456};
        for (double r : radii) {
            double area = computeArea(r);
            System.out.printf("Radius: %.6f -> Area: %.6f%n", r, area);
        }
    }
}