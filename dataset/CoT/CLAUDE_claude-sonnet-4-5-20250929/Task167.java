
public class Task167 {
    public static double cubeDiagonal(double volume) {
        // Input validation
        if (volume <= 0 || Double.isNaN(volume) || Double.isInfinite(volume)) {
            throw new IllegalArgumentException("Volume must be a positive finite number");
        }
        
        // Calculate side length from volume
        double side = Math.cbrt(volume);
        
        // Calculate main diagonal: side * sqrt(3)
        double diagonal = side * Math.sqrt(3);
        
        // Round to 2 decimal places
        return Math.round(diagonal * 100.0) / 100.0;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + cubeDiagonal(8));           // Expected: 3.46
        System.out.println("Test 2: " + cubeDiagonal(343));         // Expected: 12.12
        System.out.println("Test 3: " + cubeDiagonal(1157.625));    // Expected: 18.19
        System.out.println("Test 4: " + cubeDiagonal(1));           // Expected: 1.73
        System.out.println("Test 5: " + cubeDiagonal(27));          // Expected: 5.20
    }
}
