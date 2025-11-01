import java.util.Random;

public class Lse25 {

    private static final Random rand = new Random();

    /**
     * Returns a random number between 0 and 100000.
     * @return an integer representing monthly sales.
     */
    public static int getMonthlySales() {
        return rand.nextInt(100001); // Generates a random number from 0 to 100000
    }

    /**
     * Simulates a year of sales, calculates the total for each quarter,
     * and prints the results.
     */
    public static void calculateAndPrintQuarterlySales() {
        int quarter1Sales = 0;
        int quarter2Sales = 0;
        int quarter3Sales = 0;
        int quarter4Sales = 0;

        // Loop through 12 months of the year
        for (int month = 1; month <= 12; month++) {
            int monthlySale = getMonthlySales();
            if (month <= 3) {
                quarter1Sales += monthlySale;
            } else if (month <= 6) {
                quarter2Sales += monthlySale;
            } else if (month <= 9) {
                quarter3Sales += monthlySale;
            } else {
                quarter4Sales += monthlySale;
            }
        }

        // Print the results for each quarter
        System.out.println("Quarter 1 Sales: " + quarter1Sales);
        System.out.println("Quarter 2 Sales: " + quarter2Sales);
        System.out.println("Quarter 3 Sales: " + quarter3Sales);
        System.out.println("Quarter 4 Sales: " + quarter4Sales);
        System.out.println("--------------------");
    }

    public static void main(String[] args) {
        // Run 5 test cases
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1));
            calculateAndPrintQuarterlySales();
        }
    }
}