
public class Lse26 {
    // Simulates retrieving monthly sales data
    // In production, this would connect to a database or API with proper validation
    private static double getMonthlySales(int month) {
        // Input validation: ensure month is within expected range
        if (month < 0 || month > 2) {
            // Fail closed with clear error for invalid month index
            throw new IllegalArgumentException("Month index must be between 0 and 2");
        }
        
        // Simulated sales data for demonstration
        // In production, ensure data source is authenticated and validated
        double[] salesData = {15000.50, 18500.75, 22300.25};
        return salesData[month];
    }
    
    public static double calculateQuarterlySales() {
        double sum = 0.0;
        
        // Iterate exactly 3 times for first quarter (months 0, 1, 2)
        for (int i = 0; i < 3; i++) {
            // Each iteration calls getMonthlySales with validated input
            double monthlySale = getMonthlySales(i);
            
            // Check for overflow before addition (defensive programming)
            if (Double.isInfinite(sum + monthlySale)) {
                throw new ArithmeticException("Sum overflow detected");
            }
            
            sum += monthlySale;
        }
        
        return sum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal quarterly calculation
        try {
            double result = calculateQuarterlySales();
            System.out.println("Test 1 - Quarterly Sales: " + result);
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Verify individual month retrieval
        try {
            double month0 = getMonthlySales(0);
            System.out.println("Test 2 - Month 0 Sales: " + month0);
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Verify boundary condition (month 2)
        try {
            double month2 = getMonthlySales(2);
            System.out.println("Test 3 - Month 2 Sales: " + month2);
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Invalid month index (should fail)
        try {
            double invalid = getMonthlySales(3);
            System.out.println("Test 4 - Should not reach here: " + invalid);
        } catch (Exception e) {
            System.out.println("Test 4 - Expected error caught: " + e.getMessage());
        }
        
        // Test case 5: Invalid negative month index (should fail)
        try {
            double invalid = getMonthlySales(-1);
            System.out.println("Test 5 - Should not reach here: " + invalid);
        } catch (Exception e) {
            System.out.println("Test 5 - Expected error caught: " + e.getMessage());
        }
    }
}
