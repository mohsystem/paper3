
import java.util.Arrays;
import java.util.List;

public class Task32 {
    public static int calculateYears(double principal, double interestRate, double taxRate, double desired) {
        // Input validation
        if (principal < 0 || interestRate < 0 || taxRate < 0 || desired < 0) {
            throw new IllegalArgumentException("All parameters must be non-negative");
        }
        if (interestRate > 1 || taxRate > 1) {
            throw new IllegalArgumentException("Interest and tax rates must be between 0 and 1");
        }
        if (Double.isNaN(principal) || Double.isNaN(interestRate) || 
            Double.isNaN(taxRate) || Double.isNaN(desired)) {
            throw new IllegalArgumentException("Parameters cannot be NaN");
        }
        if (Double.isInfinite(principal) || Double.isInfinite(interestRate) || 
            Double.isInfinite(taxRate) || Double.isInfinite(desired)) {
            throw new IllegalArgumentException("Parameters cannot be infinite");
        }
        
        // If desired is less than or equal to principal, return 0
        if (desired <= principal) {
            return 0;
        }
        
        // Calculate years needed
        int years = 0;
        double currentAmount = principal;
        final int MAX_YEARS = 100000;
        
        while (currentAmount < desired && years < MAX_YEARS) {
            double interest = currentAmount * interestRate;
            double taxOnInterest = interest * taxRate;
            double netInterest = interest - taxOnInterest;
            currentAmount += netInterest;
            years++;
        }
        
        if (years >= MAX_YEARS) {
            throw new ArithmeticException("Maximum iteration limit reached");
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        List<TestCase> testCases = Arrays.asList(
            new TestCase(1000.0, 0.05, 0.18, 1100.0, 3),
            new TestCase(1000.0, 0.01, 0.18, 1000.0, 0),
            new TestCase(1000.0, 0.05, 0.18, 1000.0, 0),
            new TestCase(5000.0, 0.02, 0.18, 5100.0, 2),
            new TestCase(100.0, 0.10, 0.25, 200.0, 10)
        );
        
        for (int i = 0; i < testCases.size(); i++) {
            TestCase tc = testCases.get(i);
            try {
                int result = calculateYears(tc.principal, tc.interestRate, tc.taxRate, tc.desired);
                System.out.println("Test " + (i + 1) + ": Expected " + tc.expected + 
                                 ", Got " + result + " - " + 
                                 (result == tc.expected ? "PASS" : "FAIL"));
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + ": Exception - " + e.getMessage());
            }
        }
    }
    
    private static class TestCase {
        double principal;
        double interestRate;
        double taxRate;
        double desired;
        int expected;
        
        TestCase(double principal, double interestRate, double taxRate, double desired, int expected) {
            this.principal = principal;
            this.interestRate = interestRate;
            this.taxRate = taxRate;
            this.desired = desired;
            this.expected = expected;
        }
    }
}
