import java.util.Arrays;

public class Task32 {
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (Double.isNaN(principal) || Double.isNaN(interest) || Double.isNaN(tax) || Double.isNaN(desired)) return -1;
        if (Double.isInfinite(principal) || Double.isInfinite(interest) || Double.isInfinite(tax) || Double.isInfinite(desired)) return -1;

        if (desired <= principal) return 0;

        double netRate = interest * (1.0 - tax);
        if (netRate <= 0.0) return -1;

        int years = 0;
        final int MAX_YEARS = 10_000_000;
        double p = principal;

        while (p < desired) {
            double interestEarned = p * interest;
            double afterTaxInterest = interestEarned * (1.0 - tax);
            p += afterTaxInterest;
            years++;
            if (years > MAX_YEARS || Double.isNaN(p) || Double.isInfinite(p)) return -1;
        }
        return years;
    }

    public static void main(String[] args) {
        double[][] tests = {
            {1000.0, 0.05, 0.18, 1100.0},
            {1000.0, 0.05, 0.18, 1000.0},
            {1000.0, 0.0,  0.18, 1100.0},
            {1200.0, 0.0175, 0.05, 1300.0},
            {1500.0, 0.07, 0.6, 2000.0}
        };
        for (int i = 0; i < tests.length; i++) {
            int years = calculateYears(tests[i][0], tests[i][1], tests[i][2], tests[i][3]);
            System.out.println("Test " + (i + 1) + ": years = " + years);
        }
    }
}