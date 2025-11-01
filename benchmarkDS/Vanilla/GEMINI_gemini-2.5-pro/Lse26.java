public class Lse26 {

    /**
     * A mock function to get sales for a given month (0-indexed) from a data source.
     * @param month The month index (0 for Jan, 1 for Feb, etc.).
     * @param salesData An array containing monthly sales data.
     * @return The sales amount for the given month.
     */
    public static double getMonthlySales(int month, double[] salesData) {
        // In a real application, this might fetch data from a database or a file.
        if (month >= 0 && month < salesData.length) {
            return salesData[month];
        }
        return 0.0; // Return 0 if the month is out of bounds
    }

    /**
     * Calculates the total sales for the first quarter (first 3 months).
     * @param salesData An array containing at least 3 months of sales data.
     * @return The total sales for the first quarter.
     */
    public static double calculateFirstQuarterSales(double[] salesData) {
        double sum = 0.0;
        // The for loop iterates 3 times starting at 0 and ending at 2.
        for (int i = 0; i < 3; i++) {
            // On each iteration the loop calls getMonthlySales with the current value of i.
            double monthlySale = getMonthlySales(i, salesData);
            // The return value of getMonthlySales is added to sum.
            sum += monthlySale;
        }
        // After the loop; sum contains the total sales for the first quarter.
        return sum;
    }

    public static void main(String[] args) {
        System.out.println("--- Java: 5 Test Cases ---");

        // Test Case 1
        double[] sales1 = {1000.50, 1200.75, 1100.25, 1300.00};
        double total1 = calculateFirstQuarterSales(sales1);
        System.out.printf("Test Case 1: First quarter sales: %.2f%n", total1);

        // Test Case 2
        double[] sales2 = {2500.00, 2600.00, 2700.00, 2800.00};
        double total2 = calculateFirstQuarterSales(sales2);
        System.out.printf("Test Case 2: First quarter sales: %.2f%n", total2);

        // Test Case 3
        double[] sales3 = {99.99, 100.01, 250.50, 300.00};
        double total3 = calculateFirstQuarterSales(sales3);
        System.out.printf("Test Case 3: First quarter sales: %.2f%n", total3);

        // Test Case 4: All zero sales
        double[] sales4 = {0.0, 0.0, 0.0, 100.0};
        double total4 = calculateFirstQuarterSales(sales4);
        System.out.printf("Test Case 4: First quarter sales: %.2f%n", total4);

        // Test Case 5: Large numbers
        double[] sales5 = {1500000.78, 2300000.45, 1950000.12, 2100000.00};
        double total5 = calculateFirstQuarterSales(sales5);
        System.out.printf("Test Case 5: First quarter sales: %.2f%n", total5);
    }
}