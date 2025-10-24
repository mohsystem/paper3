
public class Task32 {
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        // Input validation
        if (principal <= 0 || desired <= 0 || interest < 0 || interest > 1 || tax < 0 || tax > 1) {
            throw new IllegalArgumentException("Invalid input parameters");
        }
        
        // Edge case: desired sum already reached
        if (desired <= principal) {
            return 0;
        }
        
        // Safety limit to prevent infinite loops
        final int MAX_YEARS = 1000;
        int years = 0;
        double currentPrincipal = principal;
        
        while (currentPrincipal < desired && years < MAX_YEARS) {
            double yearInterest = currentPrincipal * interest;
            double afterTax = yearInterest * (1 - tax);
            currentPrincipal += afterTax;
            years++;
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        // Test case 1: Example from problem
        System.out.println("Test 1: " + calculateYears(1000, 0.05, 0.18, 1100) + " years (Expected: 3)");
        
        // Test case 2: Already at desired sum
        System.out.println("Test 2: " + calculateYears(1000, 0.05, 0.18, 1000) + " years (Expected: 0)");
        
        // Test case 3: Higher interest rate
        System.out.println("Test 3: " + calculateYears(1000, 0.10, 0.18, 1200) + " years (Expected: 3)");
        
        // Test case 4: Lower tax rate
        System.out.println("Test 4: " + calculateYears(1000, 0.05, 0.10, 1100) + " years (Expected: 2)");
        
        // Test case 5: Larger gap
        System.out.println("Test 5: " + calculateYears(1000, 0.05, 0.18, 2000) + " years (Expected: 17)");
    }
}
