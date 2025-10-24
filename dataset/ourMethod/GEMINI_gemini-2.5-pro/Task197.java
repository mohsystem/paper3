import java.util.Random;
import java.util.Arrays;

public class Task197 {

    static class Solution {
        private final double radius;
        private final double x_center;
        private final double y_center;
        private final Random random;

        public Solution(double radius, double x_center, double y_center) {
            this.radius = radius;
            this.x_center = x_center;
            this.y_center = y_center;
            this.random = new Random();
        }

        public double[] randPoint() {
            double angle = 2 * Math.PI * random.nextDouble();
            // Use Math.sqrt for uniform distribution over the area of the circle
            double r = radius * Math.sqrt(random.nextDouble());
            
            double x = x_center + r * Math.cos(angle);
            double y = y_center + r * Math.sin(angle);
            
            return new double[]{x, y};
        }
    }

    public static void main(String[] args) {
        // Test Case 1
        Solution sol1 = new Solution(1.0, 0.0, 0.0);
        System.out.println("Test Case 1 (R=1, C=(0,0)):");
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol1.randPoint()));
        }
        System.out.println();

        // Test Case 2
        Solution sol2 = new Solution(10.0, 5.0, -5.0);
        System.out.println("Test Case 2 (R=10, C=(5,-5)):");
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol2.randPoint()));
        }
        System.out.println();
        
        // Test Case 3
        Solution sol3 = new Solution(0.1, 100.0, 100.0);
        System.out.println("Test Case 3 (R=0.1, C=(100,100)):");
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol3.randPoint()));
        }
        System.out.println();

        // Test Case 4
        Solution sol4 = new Solution(1000.0, -500.0, 200.0);
        System.out.println("Test Case 4 (R=1000, C=(-500,200)):");
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol4.randPoint()));
        }
        System.out.println();

        // Test Case 5
        Solution sol5 = new Solution(5.0, 0.0, 0.0);
        System.out.println("Test Case 5 (R=5, C=(0,0)):");
        for (int i = 0; i < 5; i++) {
            System.out.println(Arrays.toString(sol5.randPoint()));
        }
        System.out.println();
    }
}