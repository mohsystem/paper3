
import java.util.Random;

class Solution {
    private double radius;
    private double xCenter;
    private double yCenter;
    // Security: Use java.security.SecureRandom for better randomness
    private Random random;
    
    // Constructor: initialize with circle parameters
    // Security: Validate input parameters to prevent invalid state
    public Solution(double radius, double xCenter, double yCenter) {
        // Input validation: ensure parameters meet constraints
        if (radius <= 0.0 || radius > 1e8) {
            throw new IllegalArgumentException("Radius must be positive and <= 10^8");
        }
        
        if (xCenter < -1e7 || xCenter > 1e7 || yCenter < -1e7 || yCenter > 1e7) {
            throw new IllegalArgumentException("Center coordinates must be in [-10^7, 10^7]");
        }
        
        this.radius = radius;
        this.xCenter = xCenter;
        this.yCenter = yCenter;
        this.random = new Random();
    }
    
    // Generate a uniform random point inside the circle
    // Security: Proper bounds checking and random number usage
    public double[] randPoint() {
        // Generate random angle in [0, 2*PI)
        double angle = random.nextDouble() * 2.0 * Math.PI;
        
        // Generate random radius with uniform distribution
        // Use sqrt to ensure uniform area distribution
        double r = Math.sqrt(random.nextDouble()) * radius;
        
        // Convert polar to Cartesian coordinates
        // Bounds are safe as r <= radius and angle is normalized
        double x = xCenter + r * Math.cos(angle);
        double y = yCenter + r * Math.sin(angle);
        
        return new double[]{x, y};
    }
}

public class Task197 {
    public static void main(String[] args) {
        System.out.println("Test Case 1: Circle with radius 1.0, center (0.0, 0.0)");
        try {
            Solution sol1 = new Solution(1.0, 0.0, 0.0);
            for (int i = 0; i < 3; i++) {
                double[] point = sol1.randPoint();
                System.out.printf("  Point %d: [%.5f, %.5f]%n", i + 1, point[0], point[1]);
            }
        } catch (IllegalArgumentException e) {
            System.out.println("  Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 2: Circle with radius 10.0, center (5.0, -7.5)");
        try {
            Solution sol2 = new Solution(10.0, 5.0, -7.5);
            for (int i = 0; i < 3; i++) {
                double[] point = sol2.randPoint();
                System.out.printf("  Point %d: [%.5f, %.5f]%n", i + 1, point[0], point[1]);
            }
        } catch (IllegalArgumentException e) {
            System.out.println("  Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Circle with radius 0.01, center (0.0, 0.0)");
        try {
            Solution sol3 = new Solution(0.01, 0.0, 0.0);
            for (int i = 0; i < 3; i++) {
                double[] point = sol3.randPoint();
                System.out.printf("  Point %d: [%.7f, %.7f]%n", i + 1, point[0], point[1]);
            }
        } catch (IllegalArgumentException e) {
            System.out.println("  Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Circle with radius 100.0, center (-50.0, 50.0)");
        try {
            Solution sol4 = new Solution(100.0, -50.0, 50.0);
            for (int i = 0; i < 3; i++) {
                double[] point = sol4.randPoint();
                System.out.printf("  Point %d: [%.5f, %.5f]%n", i + 1, point[0], point[1]);
            }
        } catch (IllegalArgumentException e) {
            System.out.println("  Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 5: Invalid input (negative radius)");
        try {
            Solution sol5 = new Solution(-1.0, 0.0, 0.0);
            System.out.println("  Should have thrown exception");
        } catch (IllegalArgumentException e) {
            System.out.println("  Correctly rejected: " + e.getMessage());
        }
    }
}
