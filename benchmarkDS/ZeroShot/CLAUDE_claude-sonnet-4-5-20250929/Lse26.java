
public class Lse26 {
    public static double getMonthlySales(int month) {
        // Simulated monthly sales data (in a real application, this might come from a database)
        double[] salesData = {15000.50, 22000.75, 18500.25, 25000.00, 30000.50, 
                              27000.75, 32000.25, 28000.50, 24000.75, 26000.25, 
                              31000.50, 35000.75};
        
        // Validate month index
        if (month >= 0 && month < salesData.length) {
            return salesData[month];
        }
        return 0.0;
    }
    
    public static double calculateFirstQuarterSales() {
        double sum = 0.0;
        
        for (int i = 0; i < 3; i++) {
            sum += getMonthlySales(i);
        }
        
        return sum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Calculate first quarter sales
        System.out.println("Test Case 1 - First Quarter Sales: $" + calculateFirstQuarterSales());
        
        // Test case 2: Verify individual months
        System.out.println("\\nTest Case 2 - Individual Month Sales:");
        for (int i = 0; i < 3; i++) {
            System.out.println("Month " + i + ": $" + getMonthlySales(i));
        }
        
        // Test case 3: Manual calculation verification
        System.out.println("\\nTest Case 3 - Manual Verification:");
        double manual = getMonthlySales(0) + getMonthlySales(1) + getMonthlySales(2);
        System.out.println("Manual calculation: $" + manual);
        
        // Test case 4: Check with different starting data
        System.out.println("\\nTest Case 4 - Second Quarter Sales:");
        double q2Sum = 0.0;
        for (int i = 3; i < 6; i++) {
            q2Sum += getMonthlySales(i);
        }
        System.out.println("Second Quarter Sales: $" + q2Sum);
        
        // Test case 5: Validate boundary conditions
        System.out.println("\\nTest Case 5 - Boundary Conditions:");
        System.out.println("Invalid month (-1): $" + getMonthlySales(-1));
        System.out.println("Invalid month (12): $" + getMonthlySales(12));
        System.out.println("Valid month (0): $" + getMonthlySales(0));
    }
}
