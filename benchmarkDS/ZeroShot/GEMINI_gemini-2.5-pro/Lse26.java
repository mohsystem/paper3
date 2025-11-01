public class Lse26 {

    /**
     * Retrieves the sales for a specific month from a data source.
     * Secure: Performs bounds checking to prevent ArrayOutOfBoundsException.
     *
     * @param monthIndex The index of the month (0-2).
     * @param salesData  An array containing monthly sales data. Must not be null.
     * @return The sales for the given month, or 0L if the index is invalid or data is null.
     */
    public static long getMonthlySales(int monthIndex, long[] salesData) {
        if (salesData == null || monthIndex < 0 || monthIndex >= salesData.length) {
            // Handle invalid input gracefully by returning 0.
            return 0L;
        }
        return salesData[monthIndex];
    }

    /**
     * Calculates the total sales for the first quarter (first 3 months).
     * The for loop iterates 3 times. On each iteration, it calls getMonthlySales
     * with the current loop index i. The return value is added to a running sum.
     *
     * @param firstQuarterSalesData An array with at least 3 elements representing sales for Jan, Feb, Mar.
     * @return The sum of the first three months' sales.
     */
    public static long calculateFirstQuarterSales(long[] firstQuarterSalesData) {
        // Use long for sum to prevent potential integer overflow with large sales figures.
        long sum = 0L;
        
        // The loop iterates 3 times for the first quarter (months 0, 1, 2).
        for (int i = 0; i < 3; i++) {
            // On each iteration, get the sales for the month and add to sum.
            sum += getMonthlySales(i, firstQuarterSalesData);
        }
        
        return sum;
    }

    public static void main(String[] args) {
        // 5 test cases to validate the functionality.
        long[][] testCases = {
            {10000L, 12000L, 15000L},             // Standard case
            {0L, 0L, 0L},                         // All zero sales
            {9000000000000000000L, 10L, 20L},     // Large value test to check overflow prevention
            {1500L, 2500L, 500L},                 // Small values
            {-100L, 500L, -200L}                  // Edge case with negative values (e.g., returns)
        };

        System.out.println("Running 5 Java test cases...");
        for (int i = 0; i < testCases.length; i++) {
            long[] currentCase = testCases[i];
            long totalSales = calculateFirstQuarterSales(currentCase);
            System.out.printf("Test Case %d: [%d, %d, %d] -> Total Sales: %d%n",
                              i + 1, currentCase[0], currentCase[1], currentCase[2], totalSales);
        }
    }
}