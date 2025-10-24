
public class Task32 {
    public static int calculateYears(double principal, double interest, double tax, double desired) {
        if (principal >= desired) {
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
        // Test case 1: Example from problem
        System.out.println("Test 1: " + calculateYears(1000.00, 0.05, 0.18, 1100.00) + " (Expected: 3)");
        
        // Test case 2: Principal equals desired
        System.out.println("Test 2: " + calculateYears(1000.00, 0.05, 0.18, 1000.00) + " (Expected: 0)");
        
        // Test case 3: Small growth needed
        System.out.println("Test 3: " + calculateYears(1000.00, 0.10, 0.10, 1050.00) + " (Expected: 1)");
        
        // Test case 4: Large growth needed
        System.out.println("Test 4: " + calculateYears(1000.00, 0.01, 0.18, 1200.00) + " (Expected: 25)");
        
        // Test case 5: High interest, low tax
        System.out.println("Test 5: " + calculateYears(1000.00, 0.20, 0.10, 2000.00) + " (Expected: 4)");
    }
}
