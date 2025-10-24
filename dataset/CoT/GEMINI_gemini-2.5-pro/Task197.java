import java.util.Random;
import java.util.Arrays;

public class Task197 {
    private double radius;
    private double x_center;
    private double y_center;
    private Random random;

    public Task197(double radius, double x_center, double y_center) {
        this.radius = radius;
        this.x_center = x_center;
        this.y_center = y_center;
        this.random = new Random();
    }

    public double[] randPoint() {
        // Generate a random angle and a random length, then convert to cartesian
        // The square root is necessary for uniform distribution over the area
        double angle = random.nextDouble() * 2 * Math.PI;
        double length = Math.sqrt(random.nextDouble()) * this.radius;
        
        double x = this.x_center + length * Math.cos(angle);
        double y = this.y_center + length * Math.sin(angle);
        
        return new double[]{x, y};
    }

    public static void main(String[] args) {
        System.out.println("Java Output:");
        
        // Test Case 1: Unit circle at origin
        System.out.println("\nTest Case 1: radius=1.0, center=(0.0, 0.0)");
        Task197 sol1 = new Task197(1.0, 0.0, 0.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol1.randPoint()));
        }

        // Test Case 2: Larger circle with offset center
        System.out.println("\nTest Case 2: radius=10.0, center=(5.0, -5.0)");
        Task197 sol2 = new Task197(10.0, 5.0, -5.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol2.randPoint()));
        }
        
        // Test Case 3: Small radius
        System.out.println("\nTest Case 3: radius=0.1, center=(1.0, 1.0)");
        Task197 sol3 = new Task197(0.1, 1.0, 1.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol3.randPoint()));
        }

        // Test Case 4: Large radius and coordinates
        System.out.println("\nTest Case 4: radius=1000.0, center=(-100.0, 200.0)");
        Task197 sol4 = new Task197(1000.0, -100.0, 200.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol4.randPoint()));
        }
        
        // Test Case 5: Zero center, different radius
        System.out.println("\nTest Case 5: radius=5.0, center=(0.0, 0.0)");
        Task197 sol5 = new Task197(5.0, 0.0, 0.0);
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol5.randPoint()));
        }
    }
}