public class Lse27 {

    /**
     * Calculates the total sales over three months.
     * @param month1Sales Sales for the first month.
     * @param month2Sales Sales for the second month.
     * @param month3Sales Sales for the third month.
     * @return The sum of the three months' sales.
     */
    public static double calculateTotalSales(double month1Sales, double month2Sales, double month3Sales) {
        // The first line declares a variable called totalSales.
        double totalSales = 0.0;
        // The next three lines add the sales for each month to the totalSales variable.
        totalSales += month1Sales;
        totalSales += month2Sales;
        totalSales += month3Sales;
        return totalSales;
    }

    public static void main(String[] args) {
        // 5 Test Cases

        // Test Case 1
        double sales1 = calculateTotalSales(1500.50, 2200.75, 1850.25);
        // The last line prints the total sales to the screen.
        System.out.println("Test Case 1 Total Sales: " + String.format("%.2f", sales1));

        // Test Case 2
        double sales2 = calculateTotalSales(0.0, 0.0, 0.0);
        System.out.println("Test Case 2 Total Sales: " + String.format("%.2f", sales2));

        // Test Case 3
        double sales3 = calculateTotalSales(99999.99, 123456.78, 87654.32);
        System.out.println("Test Case 3 Total Sales: " + String.format("%.2f", sales3));

        // Test Case 4
        double sales4 = calculateTotalSales(100.0, 200.0, 300.0);
        System.out.println("Test Case 4 Total Sales: " + String.format("%.2f", sales4));
        
        // Test Case 5
        double sales5 = calculateTotalSales(543.21, 654.32, 765.43);
        System.out.println("Test Case 5 Total Sales: " + String.format("%.2f", sales5));
    }
}