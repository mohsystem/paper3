public class Task167 {

    /**
     * Calculates the length of a cube's main diagonal from its volume.
     *
     * @param volume The volume of the cube. Must be a positive number.
     * @return The length of the main diagonal, rounded to two decimal places.
     *         Returns 0.0 for non-positive volume.
     */
    public static double cubeDiagonal(double volume) {
        // Secure coding: Validate input to ensure it's within the valid domain.
        if (volume <= 0) {
            return 0.0;
        }

        // V = s^3  =>  s = cbrt(V)
        double side = Math.cbrt(volume);

        // Diagonal d = s * sqrt(3)
        double diagonal = side * Math.sqrt(3);

        // Round the result to two decimal places.
        return Math.round(diagonal * 100.0) / 100.0;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(String.format("%.2f", cubeDiagonal(8)));
        System.out.println(String.format("%.2f", cubeDiagonal(343)));
        System.out.println(String.format("%.2f", cubeDiagonal(1157.625)));
        System.out.println(String.format("%.2f", cubeDiagonal(1)));
        System.out.println(String.format("%.2f", cubeDiagonal(27)));
    }
}