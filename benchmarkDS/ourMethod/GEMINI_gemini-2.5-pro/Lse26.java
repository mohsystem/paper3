public class Lse26 {

    /**
     * Simulates fetching monthly sales data.
     *
     * @param month The month index (0 for January, 1 for February, etc.).
     * @param monthlySalesData An array containing sales data for each month.
     * @return The sales for the given month. Returns 0.0 if the month is out of bounds.
     */
    private static double getMonthlySales(int month, double[] monthlySalesData) {
        // Input validation: ensure the month index is within the bounds of the data array.
        if (monthlySalesData != null && month >= 0 && month < monthlySalesData.length) {
            return monthlySalesData[month];
        }
        return 0.0; // Fail safe by returning 0 if input is invalid
    }

    /**
     * Calculates the total sales for the first quarter (first 3 months).
     *
     * @param salesData An array containing at least 3 months of sales data.
     * @return The total sales for the first quarter.
     */
    public static double calculateFirstQuarterSales(double[] salesData) {
        // Input validation: ensure salesData is not null and has enough data for a quarter.
        if (salesData == null || salesData.length < 3) {
            // Return 0 or throw an exception. Returning 0 is a safer default.
            return 0.0;
        }

        double sum = 0.0;
        // The for loop iterates 3 times for the first quarter (months 0, 1, 2).
        for (int i = 0; i < 3; i++) {
            // On each iteration, the loop calls getMonthlySales with the current value of i.
            // The return value is added to sum.
            sum += getMonthlySales(i, salesData);
        }
        // After the loop, sum contains the total sales for the first quarter.
        return sum;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("--- 5 Test Cases ---");

        // Test Case 1: Standard sales data
        double[] sales1 = {1500.50, 2200.00, 1850.75, 3000.00};
        System.out.printf("Test Case 1 Total: %.2f%n", calculateFirstQuarterSales(sales1)); // Expected: 5551.25

        // Test Case 2: Sales data with zeros
        double[] sales2 = {1000.00, 0.00, 500.00};
        System.out.printf("Test Case 2 Total: %.2f%n", calculateFirstQuarterSales(sales2)); // Expected: 1500.00

        // Test Case 3: High value sales data
        double[] sales3 = {100000.00, 150000.50, 125000.25, 90000.00};
        System.out.printf("Test Case 3 Total: %.2f%n", calculateFirstQuarterSales(sales3)); // Expected: 375000.75

        // Test Case 4: Insufficient data (less than 3 months)
        double[] sales4 = {500.00, 600.00};
        System.out.printf("Test Case 4 Total: %.2f%n", calculateFirstQuarterSales(sales4)); // Expected: 0.00

        // Test Case 5: Null data
        double[] sales5 = null;
        System.out.printf("Test Case 5 Total: %.2f%n", calculateFirstQuarterSales(sales5)); // Expected: 0.00
    }
}