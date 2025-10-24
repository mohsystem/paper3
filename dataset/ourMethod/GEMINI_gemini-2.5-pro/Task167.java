import java.lang.Math;

public class Task167 {

    /**
     * Calculates the length of a cube's main diagonal given its volume.
     *
     * @param volume The volume of the cube. Must be a positive number.
     * @return The length of the main diagonal, rounded to two decimal places.
     * @throws IllegalArgumentException if the volume is not a positive number.
     */
    public static double cubeDiagonal(double volume) {
        if (volume <= 0) {
            throw new IllegalArgumentException("Volume must be a positive number.");
        }
        double side = Math.cbrt(volume);
        double diagonal = side * Math.sqrt(3);
        // Round to two decimal places
        return Math.round(diagonal * 100.0) / 100.0;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(cubeDiagonal(8));

        // Test Case 2
        System.out.println(cubeDiagonal(343));

        // Test Case 3
        System.out.println(cubeDiagonal(1157.625));
        
        // Test Case 4: Unit cube
        System.out.println(cubeDiagonal(1));

        // Test Case 5: Invalid input
        try {
            System.out.println(cubeDiagonal(-1));
        } catch (IllegalArgumentException e) {
            System.out.println(e.getMessage());
        }
    }
}