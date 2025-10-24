import java.nio.charset.StandardCharsets;

public class Task32 {

    // Computes the number of years needed to reach desired amount with yearly interest and tax on interest.
    // Returns -1 for invalid inputs or if the target cannot be reached (e.g., non-positive growth when D > P).
    public static long yearsToTarget(double principal, double interestRate, double taxRate, double desired) {
        if (!Double.isFinite(principal) || !Double.isFinite(interestRate) || !Double.isFinite(taxRate) || !Double.isFinite(desired)) {
            return -1;
        }
        if (principal < 0.0 || desired < 0.0 || interestRate < 0.0 || taxRate < 0.0 || taxRate > 1.0) {
            return -1;
        }
        if (desired <= principal) {
            return 0;
        }

        double effectiveRate = interestRate * (1.0 - taxRate);
        if (effectiveRate <= 0.0) {
            return -1; // Cannot reach a higher desired amount without positive growth
        }

        long years = 0;
        double amount = principal;
        // Loop until amount >= desired. This is safe due to effectiveRate > 0.
        while (amount < desired) {
            double interest = amount * interestRate;
            double netGain = interest * (1.0 - taxRate);
            amount += netGain;
            years++;

            // Safety: guard against non-finite values to avoid infinite loops
            if (!Double.isFinite(amount)) {
                return -1;
            }
        }
        return years;
    }

    private static void runTest(double p, double i, double t, double d) {
        long y = yearsToTarget(p, i, t, d);
        System.out.println(String.format("P=%.2f I=%.4f T=%.4f D=%.2f => Years=%d", p, i, t, d, y));
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(1000.00, 0.05, 0.18, 1100.00); // Expected 3
        runTest(5000.00, 0.05, 0.20, 5000.00); // Expected 0
        runTest(1000.00, 0.05, 1.20, 1100.00); // Invalid tax, Expected -1
        runTest(1000.00, 0.00, 0.18, 1001.00); // Zero growth, Expected -1
        runTest(1500.00, 0.07, 0.25, 2000.00); // Expected 6
    }
}