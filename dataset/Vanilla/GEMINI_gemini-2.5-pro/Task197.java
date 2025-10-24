import java.util.Random;
import java.util.Arrays;

class Solution {
    private double radius;
    private double x_center;
    private double y_center;
    private Random rand;

    public Solution(double radius, double x_center, double y_center) {
        this.radius = radius;
        this.x_center = x_center;
        this.y_center = y_center;
        this.rand = new Random();
    }
    
    public double[] randPoint() {
        double angle = 2 * Math.PI * rand.nextDouble();
        double r = this.radius * Math.sqrt(rand.nextDouble());
        double x = this.x_center + r * Math.cos(angle);
        double y = this.y_center + r * Math.sin(angle);
        return new double[]{x, y};
    }
}

public class Task197 {
    public static void main(String[] args) {
        Solution s1 = new Solution(1.0, 0.0, 0.0);
        System.out.println(Arrays.toString(s1.randPoint()));
        System.out.println(Arrays.toString(s1.randPoint()));
        System.out.println(Arrays.toString(s1.randPoint()));
        
        Solution s2 = new Solution(10.0, 5.0, -7.5);
        System.out.println(Arrays.toString(s2.randPoint()));
        System.out.println(Arrays.toString(s2.randPoint()));
        
        Solution s3 = new Solution(0.01, -100.0, 100.0);
        System.out.println(Arrays.toString(s3.randPoint()));
        
        Solution s4 = new Solution(1000.0, 0.0, 0.0);
        System.out.println(Arrays.toString(s4.randPoint()));
        
        Solution s5 = new Solution(2.0, 2.0, 2.0);
        System.out.println(Arrays.toString(s5.randPoint()));
    }
}