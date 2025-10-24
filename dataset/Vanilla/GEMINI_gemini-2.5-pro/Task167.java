public class Task167 {

    /**
     * Calculates the length of a cube's main diagonal from its volume.
     *
     * @param volume The volume of the cube.
     * @return The length of the main diagonal, rounded to two decimal places.
     */
    public static double cubeDiagonal(double volume) {
        // Side of the cube = cube root of the volume
        double side = Math.cbrt(volume);
        
        // Main diagonal = side * sqrt(3)
        double diagonal = side * Math.sqrt(3);
        
        // Round the result to two decimal places
        return Math.round(diagonal * 100.0) / 100.0;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(cubeDiagonal(8));         // Expected: 3.46
        System.out.println(cubeDiagonal(343));       // Expected: 12.12
        System.out.println(cubeDiagonal(1157.625));  // Expected: 18.19
        System.out.println(cubeDiagonal(1));         // Expected: 1.73
        System.out.println(cubeDiagonal(1000));      // Expected: 17.32
    }
}