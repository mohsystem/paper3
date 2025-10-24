
import java.util.Random;

class Solution {
    private double radius;
    private double x_center;
    private double y_center;
    private Random random;
    
    public Solution(double radius, double x_center, double y_center) {
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
        Solution solution1 = new Solution(1.0, 0.0, 0.0);
        System.out.println("Test 1:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution1.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 2
        Solution solution2 = new Solution(10.0, 5.0, -7.5);
        System.out.println("\\nTest 2:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution2.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 3
        Solution solution3 = new Solution(0.01, 0.0, 0.0);
        System.out.println("\\nTest 3:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution3.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 4
        Solution solution4 = new Solution(100.0, -50.0, 50.0);
        System.out.println("\\nTest 4:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution4.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
        
        // Test case 5
        Solution solution5 = new Solution(5.0, 10.0, 10.0);
        System.out.println("\\nTest 5:");
        for (int i = 0; i < 3; i++) {
            double[] point = solution5.randPoint();
            System.out.printf("[%.5f, %.5f]%n", point[0], point[1]);
        }
    }
}

class Task197 {
    public static void main(String[] args) {
        Solution.main(args);
    }
}
