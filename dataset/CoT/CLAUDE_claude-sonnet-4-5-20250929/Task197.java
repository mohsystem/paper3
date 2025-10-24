
import java.util.Random;

class Task197 {
    private double radius;
    private double x_center;
    private double y_center;
    private Random random;
    
    public Task197(double radius, double x_center, double y_center) {
        if (radius <= 0) {
            throw new IllegalArgumentException("Radius must be positive");
        }
        this.radius = radius;
        this.x_center = x_center;
        this.y_center = y_center;
        this.random = new Random();
    }
    
    public double[] randPoint() {
        double angle = random.nextDouble() * 2 * Math.PI;
        double r = Math.sqrt(random.nextDouble()) * radius;
        
        double x = x_center + r * Math.cos(angle);
        double y = y_center + r * Math.sin(angle);
        
        return new double[]{x, y};
    }
    
    public static void main(String[] args) {
        Task197 solution1 = new Task197(1.0, 0.0, 0.0);
        System.out.println("Test 1: " + java.util.Arrays.toString(solution1.randPoint()));
        System.out.println("Test 2: " + java.util.Arrays.toString(solution1.randPoint()));
        System.out.println("Test 3: " + java.util.Arrays.toString(solution1.randPoint()));
        
        Task197 solution2 = new Task197(5.0, 2.0, 3.0);
        System.out.println("Test 4: " + java.util.Arrays.toString(solution2.randPoint()));
        System.out.println("Test 5: " + java.util.Arrays.toString(solution2.randPoint()));
    }
}
