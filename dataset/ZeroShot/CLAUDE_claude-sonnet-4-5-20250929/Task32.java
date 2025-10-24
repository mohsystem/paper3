
public class Task32 {
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (desired <= principal) {
            return 0;
        }
        
        int years = 0;
        double currentAmount = principal;
        
        while (currentAmount < desired) {
            double yearInterest = currentAmount * interest;
            double taxAmount = yearInterest * tax;
            double netInterest = yearInterest - taxAmount;
            currentAmount += netInterest;
            years++;
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        // Test case 1: Example from prompt
        System.out.println("Test 1: " + calculateYears(1000, 0.05, 0.18, 1100));
        
        // Test case 2: Desired equals principal
        System.out.println("Test 2: " + calculateYears(1000, 0.05, 0.18, 1000));
        
        // Test case 3: Higher interest rate
        System.out.println("Test 3: " + calculateYears(1000, 0.10, 0.18, 1200));
        
        // Test case 4: Lower tax rate
        System.out.println("Test 4: " + calculateYears(1500, 0.06, 0.10, 2000));
        
        // Test case 5: Large desired amount
        System.out.println("Test 5: " + calculateYears(5000, 0.07, 0.15, 10000));
    }
}
