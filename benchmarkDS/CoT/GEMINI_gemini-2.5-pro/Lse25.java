import java.util.Random;

public class Lse25 {

    /**
     * Returns a random sales figure between 0 and 100000, inclusive.
     * @return an integer representing the monthly sales.
     */
    public static int getMonthlySales() {
        Random rand = new Random();
        return rand.nextInt(100001);
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            
            int quarter1Sales = 0;
            int quarter2Sales = 0;
            int quarter3Sales = 0;
            int quarter4Sales = 0;

            // Call getMonthlySales 12 times, once for each month
            for (int month = 1; month <= 12; month++) {
                int sales = getMonthlySales();
                if (month <= 3) {
                    quarter1Sales += sales;
                } else if (month <= 6) {
                    quarter2Sales += sales;
                } else if (month <= 9) {
                    quarter3Sales += sales;
                } else {
                    quarter4Sales += sales;
                }
            }

            // Print the results for each quarter
            System.out.println("Quarter 1 Sales: " + quarter1Sales);
            System.out.println("Quarter 2 Sales: " + quarter2Sales);
            System.out.println("Quarter 3 Sales: " + quarter3Sales);
            System.out.println("Quarter 4 Sales: " + quarter4Sales);
            System.out.println();
        }
    }
}