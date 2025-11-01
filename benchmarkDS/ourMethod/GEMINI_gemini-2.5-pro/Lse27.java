public class Lse27 {

    /**
     * Calculates the total of three monthly sales figures and prints the result.
     *
     * @param month1Sales Sales for the first month.
     * @param month2Sales Sales for the second month.
     * @param month3Sales Sales for the third month.
     */
    public static void calculateAndPrintSales(double month1Sales, double month2Sales, double month3Sales) {
        // The first line declares a variable called totalSales.
        double totalSales = 0.0;

        // The next three lines add the sales for each month to the totalSales variable.
        totalSales += month1Sales;
        totalSales += month2Sales;
        totalSales += month3Sales;

        // The last line prints the total sales to the screen.
        System.out.printf("Total Sales: %.2f%n", totalSales);
    }

    public static void main(String[] args) {
        System.out.println("Running 5 Test Cases:");

        // Test Case 1: Standard values
        System.out.print("Test 1: ");
        calculateAndPrintSales(1250.50, 2400.75, 1850.25);

        // Test Case 2: All zero sales
        System.out.print("Test 2: ");
        calculateAndPrintSales(0.0, 0.0, 0.0);

        // Test Case 3: Large values
        System.out.print("Test 3: ");
        calculateAndPrintSales(999999.99, 123456.78, 555555.55);

        // Test Case 4: Negative values (e.g., returns)
        System.out.print("Test 4: ");
        calculateAndPrintSales(5000.00, -150.25, -300.75);

        // Test Case 5: Small values
        System.out.print("Test 5: ");
        calculateAndPrintSales(10.11, 25.22, 5.33);
    }
}