// Chain-of-Through process:
// 1) Problem understanding:
//    - Compute years Y for principal P to reach desired D with yearly interest I and tax T on interest only.
// 2) Security requirements:
//    - Validate inputs are finite and non-negative.
//    - Avoid infinite loops by detecting non-positive effective growth or unreachable states.
// 3) Secure coding generation:
//    - Use safe numeric checks and return -1 for invalid/unreachable scenarios.
// 4) Code review:
//    - Checks for NaN/Infinity, negative values, and effectiveRate <= 0 when P < D.
// 5) Secure code output:
//    - Final code uses defensive checks and clear logic.

public class Task32 {

    // Returns number of years to reach desired sum or -1 if unreachable/invalid.
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (!Double.isFinite(principal) || !Double.isFinite(interest) || !Double.isFinite(tax) || !Double.isFinite(desired)) {
            return -1;
        }
        if (principal < 0 || interest < 0 || tax < 0 || desired < 0) {
            return -1;
        }
        if (desired <= principal) {
            return 0;
        }

        double effectiveRate = interest * (1.0 - tax);
        if (effectiveRate <= 0.0) {
            // No growth possible while desired > principal
            return -1;
        }

        int years = 0;
        double p = principal;
        // Loop until reaching or exceeding the desired amount
        // Use a large cap to prevent pathological loops (practical safeguard)
        final int MAX_YEARS = 100_000_000;
        while (p < desired) {
            double interestEarned = p * interest;
            double afterTaxInterest = interestEarned * (1.0 - tax);
            p += afterTaxInterest;
            years++;
            if (years > MAX_YEARS) {
                return -1; // safeguard
            }
        }
        return years;
    }

    // 5 test cases
    public static void main(String[] args) {
        System.out.println(calculateYears(1000.0, 0.05, 0.18, 1100.0)); // Expected: 3
        System.out.println(calculateYears(1000.0, 0.05, 0.18, 1000.0)); // Expected: 0
        System.out.println(calculateYears(1000.0, 0.00, 0.10, 1200.0)); // Expected: -1 (unreachable)
        System.out.println(calculateYears(1500.0, 0.07, 0.20, 2000.0)); // Expected: 6
        System.out.println(calculateYears(500.0, 0.05, 0.00, 525.0));   // Expected: 1
    }
}