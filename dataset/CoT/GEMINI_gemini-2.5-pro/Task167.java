import java.lang.Math;
import java.text.DecimalFormat;

public class Task167 {

    /**
     * Calculates the length of a cube's main diagonal from its volume.
     *
     * @param volume The volume of the cube.
     * @return The length of the main diagonal, rounded to two decimal places.
     */
    public static double cubeDiagonal(double volume) {
        if (volume < 0) {
            return 0.0; // Or throw an exception for invalid input
        }
        // side = cube root of volume
        double side = Math.cbrt(volume);
        // diagonal = side * sqrt(3)
        double diagonal = side * Math.sqrt(3);
        // Round to two decimal places
        return Math.round(diagonal * 100.0) / 100.0;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: cubeDiagonal(8)");
        System.out.println("Expected: 3.46");
        System.out.println("Actual: " + cubeDiagonal(8));
        System.out.println();

        System.out.println("Test Case 2: cubeDiagonal(343)");
        System.out.println("Expected: 12.12");
        System.out.println("Actual: " + cubeDiagonal(343));
        System.out.println();

        System.out.println("Test Case 3: cubeDiagonal(1157.625)");
        System.out.println("Expected: 18.19");
        System.out.println("Actual: " + cubeDiagonal(1157.625));
        System.out.println();

        System.out.println("Test Case 4: cubeDiagonal(1)");
        System.out.println("Expected: 1.73");
        System.out.println("Actual: " + cubeDiagonal(1));
        System.out.println();
        
        System.out.println("Test Case 5: cubeDiagonal(27)");
        System.out.println("Expected: 5.20");
        System.out.println("Actual: " + cubeDiagonal(27));
        System.out.println();
    }
}