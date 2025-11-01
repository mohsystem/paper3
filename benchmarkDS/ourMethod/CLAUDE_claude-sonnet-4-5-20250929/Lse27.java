
public class Lse27 {
    // Method to calculate total sales from monthly sales
    public static double calculateTotalSales(double month1, double month2, double month3) {
        // Initialize totalSales to 0.0
        double totalSales = 0.0;
        
        // Validate inputs are non-negative (sales cannot be negative)
        if (month1 < 0 || month2 < 0 || month3 < 0) {
            throw new IllegalArgumentException("Sales values cannot be negative");
        }
        
        // Check for potential overflow before addition
        if (Double.isInfinite(month1) || Double.isInfinite(month2) || Double.isInfinite(month3)) {
            throw new IllegalArgumentException("Sales values are too large");
        }
        
        // Add each month's sales to totalSales
        totalSales += month1;
        totalSales += month2;
        totalSales += month3;
        
        // Verify result is valid
        if (Double.isInfinite(totalSales) || Double.isNaN(totalSales)) {
            throw new ArithmeticException("Total sales calculation resulted in invalid value");
        }
        
        return totalSales;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal positive values
        try {
            double result1 = calculateTotalSales(1000.50, 1500.75, 2000.25);
            System.out.println("Test 1 - Total sales: " + result1);
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Zero values
        try {
            double result2 = calculateTotalSales(0.0, 0.0, 0.0);
            System.out.println("Test 2 - Total sales: " + result2);
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Large values
        try {
            double result3 = calculateTotalSales(50000.99, 75000.50, 100000.01);
            System.out.println("Test 3 - Total sales: " + result3);
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Negative value (should fail)
        try {
            double result4 = calculateTotalSales(1000.0, -500.0, 2000.0);
            System.out.println("Test 4 - Total sales: " + result4);
        } catch (Exception e) {
            System.err.println("Test 4 failed (expected): " + e.getMessage());
        }
        
        // Test case 5: Mixed decimal values
        try {
            double result5 = calculateTotalSales(123.45, 678.90, 234.56);
            System.out.println("Test 5 - Total sales: " + result5);
        } catch (Exception e) {
            System.err.println("Test 5 failed: " + e.getMessage());
        }
    }
}
