public class Task32 {
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (principal >= desired) return 0;
        int years = 0;
        double p = principal;
        while (p < desired) {
            double accrued = p * interest;
            double afterTax = accrued * (1.0 - tax);
            p += afterTax;
            years++;
        }
        return years;
    }

    public static void main(String[] args) {
        System.out.println(calculateYears(1000.0, 0.05, 0.18, 1100.0)); // 3
        System.out.println(calculateYears(1000.0, 0.05, 0.18, 1000.0)); // 0
        System.out.println(calculateYears(1000.0, 0.05, 0.0, 1100.0));  // 2
        System.out.println(calculateYears(1000.0, 0.01625, 0.18, 1200.0)); // 14
        System.out.println(calculateYears(1500.0, 0.07, 0.60, 2000.0)); // 11
    }
}