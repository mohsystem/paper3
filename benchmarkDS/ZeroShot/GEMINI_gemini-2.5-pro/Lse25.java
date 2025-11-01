import java.util.Random;

public class Lse25 {

    /**
     * Generates a random sales figure for a month.
     * @return an integer between 0 and 100000, inclusive.
     */
    public static int getMonthlySales() {
        // Using Random for simplicity. For security-critical applications, 
        // java.security.SecureRandom would be a better choice.
        Random rand = new Random();
        // The upper bound is exclusive, so we add 1.
        return rand.nextInt(100001); 
    }

    /**
     * Calculates and prints the total sales for each of the four quarters.
     */
    public static void calculateAndPrintQuarterlySales() {
        int q1Sales = 0;
        int q2Sales = 0;
        int q3Sales = 0;
        int q4Sales = 0;

        for (int month = 1; month <= 12; month++) {
            int monthlySale = getMonthlySales();
            if (month <= 3) {
                q1Sales += monthlySale;
            } else if (month <= 6) {
                q2Sales += monthlySale;
            } else if (month <= 9) {
                q3Sales += monthlySale;
            } else {
                q4Sales += monthlySale;
            }
        }

        System.out.println("Quarter 1 Sales: $" + q1Sales);
        System.out.println("Quarter 2 Sales: $" + q2Sales);
        System.out.println("Quarter 3 Sales: $" + q3Sales);
        System.out.println("Quarter 4 Sales: $" + q4Sales);
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            calculateAndPrintQuarterlySales();
            System.out.println();
        }
    }
}