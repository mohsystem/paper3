import java.util.Random;

public class Lse25 {

    private static final int MAX_SALES = 100000;
    private static final int MONTHS_IN_YEAR = 12;

    /**
     * Generates a random monthly sales figure.
     * @param rand A Random object instance.
     * @return A random integer between 0 and 100000 (inclusive).
     */
    public static int getMonthlySales(Random rand) {
        // Generates a random number from 0 (inclusive) to the bound (exclusive),
        // so we add 1 to the max sales value.
        return rand.nextInt(MAX_SALES + 1);
    }

    /**
     * Runs a single yearly simulation: gets 12 monthly sales,
     * calculates quarterly totals, and prints the results.
     * @param rand The random number generator to use.
     */
    public static void runSimulation(Random rand) {
        int[] monthlySales = new int[MONTHS_IN_YEAR];

        // Get sales for each month
        for (int i = 0; i < MONTHS_IN_YEAR; i++) {
            monthlySales[i] = getMonthlySales(rand);
        }

        // Calculate and print quarterly sales. Using long to prevent overflow.
        long quarter1Sales = 0;
        long quarter2Sales = 0;
        long quarter3Sales = 0;
        long quarter4Sales = 0;

        for (int i = 0; i < 3; i++) {
            quarter1Sales += monthlySales[i];
        }
        for (int i = 3; i < 6; i++) {
            quarter2Sales += monthlySales[i];
        }
        for (int i = 6; i < 9; i++) {
            quarter3Sales += monthlySales[i];
        }
        for (int i = 9; i < 12; i++) {
            quarter4Sales += monthlySales[i];
        }

        System.out.println("Quarter 1 Sales: " + quarter1Sales);
        System.out.println("Quarter 2 Sales: " + quarter2Sales);
        System.out.println("Quarter 3 Sales: " + quarter3Sales);
        System.out.println("Quarter 4 Sales: " + quarter4Sales);
    }

    /**
     * Main method to run the simulation 5 times as test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // For applications requiring security, use SecureRandom instead.
        // For this simulation, Random is sufficient.
        Random rand = new Random();
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            runSimulation(rand);
            if (i < 5) {
                System.out.println();
            }
        }
    }
}