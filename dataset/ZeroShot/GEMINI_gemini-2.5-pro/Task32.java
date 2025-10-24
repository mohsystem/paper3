public class Task32 {

    /**
     * Calculates the number of years to reach a desired sum.
     *
     * @param principal The initial principal amount.
     * @param interest  The annual interest rate (e.g., 0.05 for 5%).
     * @param tax       The tax rate on interest earned (e.g., 0.18 for 18%).
     * @param desired   The desired final sum.
     * @return The number of whole years required.
     */
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (principal >= desired) {
            return 0;
        }

        int years = 0;
        double currentPrincipal = principal;

        // Assuming interest is positive and tax is less than 100% to avoid infinite loops.
        // A truly secure implementation would validate these inputs.
        while (currentPrincipal < desired) {
            double interestEarned = currentPrincipal * interest;
            double taxPaid = interestEarned * tax;
            currentPrincipal += (interestEarned - taxPaid);
            years++;
        }

        return years;
    }

    public static void main(String[] args) {
        // Test Case 1: From description
        System.out.println("Test Case 1: " + calculateYears(1000.00, 0.05, 0.18, 1100.00)); // Expected: 3

        // Test Case 2: Desired amount is already met
        System.out.println("Test Case 2: " + calculateYears(1000.00, 0.05, 0.18, 1000.00)); // Expected: 0

        // Test Case 3: Larger growth needed
        System.out.println("Test Case 3: " + calculateYears(100.00, 0.10, 0.20, 200.00)); // Expected: 10

        // Test Case 4: No tax
        System.out.println("Test Case 4: " + calculateYears(100.00, 0.10, 0.0, 150.00)); // Expected: 5

        // Test Case 5: High tax
        System.out.println("Test Case 5: " + calculateYears(1000.00, 0.08, 0.50, 1100.00)); // Expected: 3
    }
}