import java.util.Random;
import java.util.Arrays;

class Task197 {
    private double radius;
    private double x_center;
    private double y_center;
    private Random rand;

    /**
     * Initializes the object with the radius of the circle and the position of the center.
     * @param radius The radius of the circle.
     * @param x_center The x-coordinate of the center of the circle.
     * @param y_center The y-coordinate of the center of the circle.
     */
    public Task197(double radius, double x_center, double y_center) {
        this.radius = radius;
        this.x_center = x_center;
        this.y_center = y_center;
        this.rand = new Random();
    }

    /**
     * Returns a random point inside the circle.
     * The point distribution is uniform.
     * @return A random point as a double array [x, y].
     */
    public double[] randPoint() {
        // Use polar coordinates with inverse transform sampling for uniform distribution
        double r_sqrt = Math.sqrt(rand.nextDouble()); // For uniform distribution of area
        double r = radius * r_sqrt;
        double theta = rand.nextDouble() * 2 * Math.PI;

        double x = x_center + r * Math.cos(theta);
        double y = y_center + r * Math.sin(theta);

        return new double[]{x, y};
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println("Test Case 1 (radius=1.0, center=(0.0, 0.0)):");
        Task197 sol1 = new Task197(1.0, 0.0, 0.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol1.randPoint()));
        }

        // Test Case 2
        System.out.println("\nTest Case 2 (radius=10.0, center=(5.0, -7.5)):");
        Task197 sol2 = new Task197(10.0, 5.0, -7.5);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol2.randPoint()));
        }

        // Test Case 3
        System.out.println("\nTest Case 3 (radius=0.1, center=(100.0, 100.0)):");
        Task197 sol3 = new Task197(0.1, 100.0, 100.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol3.randPoint()));
        }
        
        // Test Case 4
        System.out.println("\nTest Case 4 (radius=1e8, center=(-1e7, 1e7)):");
        Task197 sol4 = new Task197(1e8, -1e7, 1e7);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol4.randPoint()));
        }
        
        // Test Case 5
        System.out.println("\nTest Case 5 (radius=1.0, center=(1.0, 1.0)):");
        Task197 sol5 = new Task197(1.0, 1.0, 1.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol5.randPoint()));
        }
    }
}