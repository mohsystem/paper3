
public class Lse26 {
    // Simulated monthly sales data
    private static final double[] MONTHLY_SALES = {
        15000.0, 18000.0, 16500.0, 20000.0, 
        17500.0, 19000.0, 21000.0, 18500.0,
        16000.0, 22000.0, 19500.0, 23000.0
    };
    
    // Secure method to get monthly sales with input validation
    public static double getMonthlySales(int month) {
        // Validate month index to prevent out-of-bounds access
        if (month < 0 || month >= MONTHLY_SALES.length) {
            throw new IllegalArgumentException("Invalid month index: " + month);
        }
        return MONTHLY_SALES[month];
    }
    
    // Calculate first quarter sales
    public static double calculateFirstQuarterSales() {
        double sum = 0.0;
        
        // Iterate through first 3 months (index 0 to 2)
        for (int i = 0; i < 3; i++) {
            double monthlySale = getMonthlySales(i);
            // Check for valid sales value
            if (monthlySale < 0) {
                throw new IllegalStateException("Sales value cannot be negative");
            }
            sum += monthlySale;
        }
        
        return sum;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: First Quarter Sales");
        System.out.println("Total: $" + calculateFirstQuarterSales());
        
        System.out.println("\\nTest Case 2: Individual Monthly Sales");
        for (int i = 0; i < 3; i++) {
            System.out.println("Month " + i + ": $" + getMonthlySales(i));
        }
        
        System.out.println("\\nTest Case 3: Verify sum calculation");
        double manual = getMonthlySales(0) + getMonthlySales(1) + getMonthlySales(2);
        System.out.println("Manual calculation: $" + manual);
        
        System.out.println("\\nTest Case 4: Valid month access");
        System.out.println("Month 5 sales: $" + getMonthlySales(5));
        
        System.out.println("\\nTest Case 5: Error handling");
        try {
            getMonthlySales(15); // Invalid month
        } catch (IllegalArgumentException e) {
            System.out.println("Caught exception: " + e.getMessage());
        }
    }
}
