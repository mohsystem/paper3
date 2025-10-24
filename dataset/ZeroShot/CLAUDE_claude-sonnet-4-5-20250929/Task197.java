
import java.util.Random;

class Task197 {
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
        // Use square root to ensure uniform distribution
        double r = Math.sqrt(random.nextDouble()) * radius;
        double theta = random.nextDouble() * 2 * Math.PI;
        
        double x = x_center + r * Math.cos(theta);
        double y = y_center + r * Math.sin(theta);
        
        return new double[]{x, y};
    }
    
    public static void main(String[] args) {
        // Test case 1
        Task197 solution1 = new Task197(1.0, 0.0, 0.0);
        System.out.println("Test case 1:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution1.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 2
        Task197 solution2 = new Task197(10.0, 5.0, -7.5);
        System.out.println("\\nTest case 2:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution2.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 3
        Task197 solution3 = new Task197(0.01, 0.0, 0.0);
        System.out.println("\\nTest case 3:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution3.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 4
        Task197 solution4 = new Task197(100.0, -50.0, 50.0);
        System.out.println("\\nTest case 4:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution4.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 5
        Task197 solution5 = new Task197(5.5, 3.3, -2.2);
        System.out.println("\\nTest case 5:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution5.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
    }
}
