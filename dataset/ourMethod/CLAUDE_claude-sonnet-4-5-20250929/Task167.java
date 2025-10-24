
import java.util.IllegalFormatException;

public class Task167 {
    /**
     * Calculates the main diagonal of a cube given its volume.
     * Formula: diagonal = side * sqrt(3), where side = volume^(1/3)
     * 
     * @param volume The volume of the cube (must be positive)
     * @return The length of the main diagonal rounded to 2 decimal places
     * @throws IllegalArgumentException if volume is not positive or is NaN/Infinite
     */
    public static double cubeDiagonal(double volume) {
        // Validate input
        if (Double.isNaN(volume) || Double.isInfinite(volume)) {
            throw new IllegalArgumentException("Volume must be a valid finite number");
        }
        if (volume <= 0) {
            throw new IllegalArgumentException("Volume must be positive");
        }
        
        // Calculate side length from volume: side = volume^(1/3)
        double side = Math.cbrt(volume);
        
        // Validate intermediate result
        if (Double.isNaN(side) || Double.isInfinite(side)) {
            throw new IllegalArgumentException("Invalid calculation result");
        }
        
        // Calculate main diagonal: diagonal = side * sqrt(3)
        double diagonal = side * Math.sqrt(3.0);
        
        // Validate result before rounding
        if (Double.isNaN(diagonal) || Double.isInfinite(diagonal)) {
            throw new IllegalArgumentException("Invalid diagonal calculation");
        }
        
        // Round to 2 decimal places
        return Math.round(diagonal * 100.0) / 100.0;
    }
    
    public static void main(String[] args) {
        // Test case 1
        try {
            double result1 = cubeDiagonal(8.0);
            System.out.println("cubeDiagonal(8) = " + result1);
            assert Math.abs(result1 - 3.46) < 0.01 : "Test 1 failed";
        } catch (Exception e) {
            System.err.println("Test 1 error: " + e.getMessage());
        }
        
        // Test case 2
        try {
            double result2 = cubeDiagonal(343.0);
            System.out.println("cubeDiagonal(343) = " + result2);
            assert Math.abs(result2 - 12.12) < 0.01 : "Test 2 failed";
        } catch (Exception e) {
            System.err.println("Test 2 error: " + e.getMessage());
        }
        
        // Test case 3
        try {
            double result3 = cubeDiagonal(1157.625);
            System.out.println("cubeDiagonal(1157.625) = " + result3);
            assert Math.abs(result3 - 18.19) < 0.01 : "Test 3 failed";
        } catch (Exception e) {
            System.err.println("Test 3 error: " + e.getMessage());
        }
        
        // Test case 4: Edge case with volume = 1
        try {
            double result4 = cubeDiagonal(1.0);
            System.out.println("cubeDiagonal(1) = " + result4);
        } catch (Exception e) {
            System.err.println("Test 4 error: " + e.getMessage());
        }
        
        // Test case 5: Invalid input (negative volume)
        try {
            double result5 = cubeDiagonal(-10.0);
            System.out.println("cubeDiagonal(-10) = " + result5);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 correctly rejected negative volume: " + e.getMessage());
        }
    }
}
